#include "chunk_generator.h"
#include "render_quad.h"
#include "renderer/vulkan/vulkan_renderer.h"

#include <ranges>

namespace Moxel
{
	ChunkBuilder::ChunkBuilder(const ChunkWorldSpecs specs)
	{
		m_specs = specs;
	}

	void ChunkBuilder::destroy_world()
	{ 
		for (const auto& chunk: std::views::values(m_meshChunks))
		{
			if (chunk == nullptr || chunk->get_chunk_mesh() == nullptr)
			{
				continue;
			}

			VulkanRenderer::queue_resource_free(chunk->get_chunk_mesh());
		}

		m_meshChunks.clear();
	}
	
	void ChunkBuilder::update(const glm::vec3 playerPosition)
	{
		const auto playerChunkPosition = world_pos_to_chunk(playerPosition);
		bool shouldGenerateData = playerChunkPosition != m_oldPlayerChunkPosition;

		// update deletion data
		m_threadPool.enqueue([this, playerChunkPosition, shouldGenerateData]
		{
			update_data_deletion_queue();

			if (shouldGenerateData)
			{
				update_mesh_deletion_queue(playerChunkPosition);
			}
		});

		// update render data
		m_threadPool.enqueue([this, playerChunkPosition, shouldGenerateData] 
		{
			if (shouldGenerateData)
			{
				update_data_generation_queue(playerChunkPosition);
			}

			std::vector<ChunkPosition> chunkDataToUpdate;
			for (int i = 0; i < MAX_CHUNKS_DATA_PER_FRAME_GENERATED; i++)
			{
				auto lock = std::unique_lock(m_worldMutex);

				if (m_dataGenerationQueue.empty())
				{
					break;
				}

				chunkDataToUpdate.push_back(m_dataGenerationQueue.front());
				m_dataGenerationQueue.pop();
			}

			for (const auto& position: chunkDataToUpdate)
			{
				m_dataChunks[position]->generate_data(position);
			}
		});

		// generate render data
		m_threadPool.enqueue([this, playerChunkPosition, shouldGenerateData]
		{
			if (shouldGenerateData)
			{
				update_mesh_generation_queue(playerChunkPosition);
			}

			std::vector<ChunkPosition> meshDataToUpdate;
			for (int i = 0; i < MAX_CHUNKS_PER_FRAME_GENERATED; i++)
			{
				auto lock = std::unique_lock(m_worldMutex);

				if (m_meshGenerationQueue.empty())
				{
					break;
				}

				meshDataToUpdate.push_back(m_meshGenerationQueue.front());
				m_meshGenerationQueue.pop();
			}

			for (const auto& position: meshDataToUpdate)
			{
				generate_chunk_mesh(position);
			}
		});

		for (const auto& [position, array]: m_requestedMeshes)
		{
			const auto vao = std::make_shared<VulkanVertexArray>(array.first, array.second);
			m_meshChunks[position] = std::make_shared<ChunkMesh>(vao);
		}
		m_requestedMeshes.clear();

		update_render_queue(playerChunkPosition);

		m_oldPlayerChunkPosition = playerChunkPosition;
	}

	void ChunkBuilder::update_data_generation_queue(const ChunkPosition playerChunkPosition)
	{
		const int totalSize = m_specs.ChunkSize * m_specs.ChunkSize * m_specs.ChunkSize;
		const int renderDistance = m_specs.RenderDistance;

		for (int z = -(renderDistance + 1) + playerChunkPosition.z; z < (renderDistance + 1) + playerChunkPosition.z; ++z)
		{
			for (int y = -(renderDistance + 1) + playerChunkPosition.y; y < (renderDistance + 1) + playerChunkPosition.y; ++y)
			{
				for (int x = -(renderDistance + 1) + playerChunkPosition.x; x < (renderDistance + 1) + playerChunkPosition.x; ++x)
				{
					auto lock = std::unique_lock(m_worldMutex);

					auto chunkPosition = ChunkPosition(x, y, z);

					if (m_dataChunks.contains(chunkPosition) || m_meshChunks.contains(chunkPosition))
					{
						continue;
					}

					const auto chunk = std::make_shared<Chunk>(totalSize);
					m_dataChunks.emplace(chunkPosition, chunk);
					m_dataGenerationQueue.emplace(chunkPosition);
				}
			}
		}
	}

	void ChunkBuilder::update_mesh_generation_queue(const ChunkPosition playerChunkPosition)
	{
		const int renderDistance = m_specs.RenderDistance;

		for (int z = -renderDistance + playerChunkPosition.z; z < renderDistance + playerChunkPosition.z; ++z)
		{
			for (int y = -renderDistance + playerChunkPosition.y; y < renderDistance + playerChunkPosition.y; ++y)
			{
				for (int x = -renderDistance + playerChunkPosition.x; x < renderDistance + playerChunkPosition.x; ++x)
				{
					auto lock = std::unique_lock(m_worldMutex);

					auto chunkPosition = ChunkPosition(x, y, z);
					if (m_meshChunks.contains(chunkPosition) == false)
					{
						const auto left = ChunkPosition(chunkPosition.x - 1, chunkPosition.y, chunkPosition.z);
						if (m_dataChunks.contains(left) == false || m_dataChunks[left]->is_processed() == false)
						{
							continue;
						}

						const auto down = ChunkPosition(chunkPosition.x, chunkPosition.y - 1, chunkPosition.z);
						if (m_dataChunks.contains(down) == false || m_dataChunks[down]->is_processed() == false)
						{
							continue;
						}

						const auto back = ChunkPosition(chunkPosition.x, chunkPosition.y, chunkPosition.z - 1);
						if (m_dataChunks.contains(back) == false || m_dataChunks[back]->is_processed() == false)
						{
							continue;
						}

						const auto right = ChunkPosition(chunkPosition.x + 1, chunkPosition.y, chunkPosition.z);
						if (m_dataChunks.contains(right) == false || m_dataChunks[right]->is_processed() == false)
						{
							continue;
						}

						const auto up = ChunkPosition(chunkPosition.x, chunkPosition.y + 1, chunkPosition.z);
						if (m_dataChunks.contains(up) == false || m_dataChunks[up]->is_processed() == false)
						{
							continue;
						}

						const auto front = ChunkPosition(chunkPosition.x, chunkPosition.y, chunkPosition.z + 1);
						if (m_dataChunks.contains(front) == false || m_dataChunks[front]->is_processed() == false)
						{
							continue;
						}

						m_meshChunks[chunkPosition] = nullptr;
						m_meshGenerationQueue.emplace(chunkPosition);
					}
				}
			}
		}
	}

	void ChunkBuilder::update_render_queue(ChunkPosition playerChunkPosition)
	{
		const int renderDistance = m_specs.RenderDistance;

		for (const auto& position: std::views::keys(m_meshChunks))
		{
			const auto xDistance = abs(position.x - playerChunkPosition.x);
			const auto yDistance = abs(position.y - playerChunkPosition.y);
			const auto zDistance = abs(position.z - playerChunkPosition.z);

			if (xDistance <= renderDistance && yDistance <= renderDistance && zDistance <= renderDistance)
			{
				if (m_meshChunks[position] == nullptr || m_meshChunks[position]->get_chunk_mesh() == nullptr)
				{
					continue;
				}

				m_renderQueue.push(position);
			}
		}
	}

	void ChunkBuilder::update_data_deletion_queue()
	{
		auto lock = std::unique_lock(m_worldMutex);

		auto chunksToErase = std::vector<ChunkPosition>();
		for (const auto& position: std::views::keys(m_dataChunks))
		{
			const auto right = ChunkPosition(position.x + 1, position.y, position.z);
			const auto up = ChunkPosition(position.x, position.y + 1, position.z);
			const auto front = ChunkPosition(position.x, position.y, position.z + 1);
			const auto left = ChunkPosition(position.x - 1, position.y, position.z);
			const auto down = ChunkPosition(position.x, position.y - 1, position.z);
			const auto back = ChunkPosition(position.x, position.y, position.z - 1);

			if (m_meshChunks.contains(right) && m_meshChunks[right] != nullptr 
				&& m_meshChunks.contains(up) && m_meshChunks[up] != nullptr 
				&& m_meshChunks.contains(front) && m_meshChunks[front] != nullptr 
				&& m_meshChunks.contains(left) && m_meshChunks[left] != nullptr 
				&& m_meshChunks.contains(down) && m_meshChunks[down] != nullptr 
				&& m_meshChunks.contains(back) && m_meshChunks[back] != nullptr)
			{
				if (m_dataChunks.contains(position) == false)
				{
					continue;
				}

				chunksToErase.emplace_back(position);
			}
		}

		for (const auto& position: chunksToErase)
		{
			m_dataChunks.erase(position);
		}
	}

	void ChunkBuilder::update_mesh_deletion_queue(const ChunkPosition playerChunkPosition)
	{
		auto lock = std::unique_lock(m_worldMutex);

		auto chunksToErase = std::vector<ChunkPosition>();
		const auto renderDistance = m_specs.RenderDistance;
		for (const auto& position: std::views::keys(m_meshChunks))
		{
			const auto xDistance = abs(position.x - playerChunkPosition.x);
			const auto yDistance = abs(position.y - playerChunkPosition.y);
			const auto zDistance = abs(position.z - playerChunkPosition.z);

			if (xDistance > renderDistance || yDistance > renderDistance || zDistance > renderDistance)
			{
				if (m_meshChunks[position] == nullptr)
				{
					continue;
				}

				chunksToErase.emplace_back(position);
			}
		}

		for (const auto& position: chunksToErase)
		{
			if (m_meshChunks[position]->get_chunk_mesh() != nullptr)
			{
				VulkanRenderer::queue_resource_free(m_meshChunks[position]->get_chunk_mesh());
			}

			m_meshChunks.erase(position);
		}
	}

	glm::vec3 ChunkBuilder::chunk_to_world_pos(const glm::vec3 chunkPosition) const
	{ 
		return chunkPosition * static_cast<float>(m_specs.ChunkSize);
	}

	ChunkPosition ChunkBuilder::world_pos_to_chunk(const glm::vec3 worldPosition) const
	{
		const auto chunkSize = static_cast<float>(m_specs.ChunkSize);

		return {
			static_cast<int>(worldPosition.x / chunkSize),
			static_cast<int>(worldPosition.y / chunkSize),
			static_cast<int>(worldPosition.z / chunkSize)
		};
	}

	bool ChunkBuilder::get_voxel(ChunkPosition position, const int x, const int y, const int z) const
	{
		const auto chunkSize = m_specs.ChunkSize;
		int actualX = x, actualY = y, actualZ = z;

		if (x < 0 || x >= chunkSize)
		{
			actualX = x < 0 ? chunkSize - 1 : 0;
			position.x += x < 0 ? -1 : 1;
		}

		if (y < 0 || y >= chunkSize)
		{
			actualY = y < 0 ? chunkSize - 1 : 0;
			position.y += y < 0 ? -1 : 1;
		}

		if (z < 0 || z >= chunkSize)
		{
			actualZ = z < 0 ? chunkSize - 1 : 0;
			position.z += z < 0 ? -1 : 1;
		}

		return m_dataChunks.at(position)->get_block(actualZ * chunkSize * chunkSize + actualY * chunkSize + actualX);
	}

	void ChunkBuilder::generate_chunk_mesh(ChunkPosition position)
	{
		// generate mesh data from chunk
		auto totalVertices = std::vector<VoxelVertex>();
		auto totalIndices = std::vector<uint32_t>();
		int indexOffset = 0;

		const int chunkSize = m_specs.ChunkSize;
		for (int z = 0; z < chunkSize; ++z)
		{
			for (int y = 0; y < chunkSize; ++y)
			{
				for (int x = 0; x < chunkSize; ++x)
				{
					const auto mainBlock = m_dataChunks.at(position)->get_block(z * chunkSize * chunkSize + y * chunkSize + x);
					if (mainBlock == false)
					{
						continue;
					}

					const auto positionOffset = glm::i32vec3(x, y, z);
					int indexQuadOffset = 0;

					const auto leftBlock = get_voxel(position, x - 1, y, z);
					const auto downBlock = get_voxel(position, x, y - 1, z);
					const auto backBlock = get_voxel(position, x, y, z - 1);
					const auto rightBlock = get_voxel(position, x + 1, y, z);
					const auto upBlock = get_voxel(position, x, y + 1, z);
					const auto frontBlock = get_voxel(position, x, y, z + 1);

					if (downBlock == false)
					{
						auto quadDown = RenderQuad(Side::Down, positionOffset);
						quadDown.add_indices_offset(indexOffset + indexQuadOffset);
						totalVertices.insert(totalVertices.end(), quadDown.get_vertices().begin(), quadDown.get_vertices().end());
						totalIndices.insert(totalIndices.end(), quadDown.get_indices().begin(), quadDown.get_indices().end());

						indexQuadOffset += 4;
					}

					if (upBlock == false)
					{
						auto quadUp = RenderQuad(Side::Up, positionOffset);
						quadUp.add_indices_offset(indexOffset + indexQuadOffset);
						totalVertices.insert(totalVertices.end(), quadUp.get_vertices().begin(), quadUp.get_vertices().end());
						totalIndices.insert(totalIndices.end(), quadUp.get_indices().begin(), quadUp.get_indices().end());

						indexQuadOffset += 4;
					}

					if (leftBlock == false)
					{
						auto quadLeft = RenderQuad(Side::Left, positionOffset);
						quadLeft.add_indices_offset(indexOffset + indexQuadOffset);
						totalVertices.insert(totalVertices.end(), quadLeft.get_vertices().begin(), quadLeft.get_vertices().end());
						totalIndices.insert(totalIndices.end(), quadLeft.get_indices().begin(), quadLeft.get_indices().end());

						indexQuadOffset += 4;
					}

					if (rightBlock == false)
					{
						auto quadRight = RenderQuad(Side::Right, positionOffset);
						quadRight.add_indices_offset(indexOffset + indexQuadOffset);
						totalVertices.insert(totalVertices.end(), quadRight.get_vertices().begin(), quadRight.get_vertices().end());
						totalIndices.insert(totalIndices.end(), quadRight.get_indices().begin(), quadRight.get_indices().end());

						indexQuadOffset += 4;
					}

					if (backBlock == false)
					{
						auto quadBack = RenderQuad(Side::Back, positionOffset);
						quadBack.add_indices_offset(indexOffset + indexQuadOffset);
						totalVertices.insert(totalVertices.end(), quadBack.get_vertices().begin(), quadBack.get_vertices().end());
						totalIndices.insert(totalIndices.end(), quadBack.get_indices().begin(), quadBack.get_indices().end());

						indexQuadOffset += 4;
					}

					if (frontBlock == false)
					{
						auto quadFront = RenderQuad(Side::Front, positionOffset);
						quadFront.add_indices_offset(indexOffset + indexQuadOffset);
						totalVertices.insert(totalVertices.end(), quadFront.get_vertices().begin(), quadFront.get_vertices().end());
						totalIndices.insert(totalIndices.end(), quadFront.get_indices().begin(), quadFront.get_indices().end());

						indexQuadOffset += 4;
					}

					indexOffset += indexQuadOffset;
				}
			}
		}

		if (totalIndices.empty())
		{
			m_meshChunks[position] = std::make_shared<ChunkMesh>(nullptr);
			return;
		}

		m_requestedMeshes[position] = std::pair(totalIndices, totalVertices);
	}
}