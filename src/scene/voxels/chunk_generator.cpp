#include "chunk_generator.h"
#include "render_quad.h"
#include "engine/renderer/vulkan_renderer.h"

#include <ranges>

namespace Moxel
{
	ChunkBuilder::ChunkBuilder(const ChunkWorldSpecs specs)
	{
		m_specs = specs;
	}

	void ChunkBuilder::destroy_world()
	{ 
		for (const auto& chunk: m_meshChunks | std::views::values)
		{
			if (chunk == nullptr || chunk->get_chunk_mesh() == nullptr)
			{
				continue;
			}

			VulkanRenderer::queue_vao_free(chunk->get_chunk_mesh());
		}

		m_meshChunks.clear();
	}

	int ChunkBuilder::get_total_chunks_data_count() const
	{
		return m_dataChunks.size();
	}

	int ChunkBuilder::get_total_chunks_mesh_count()
	{
		int meshes = 0;
		for (const auto& mesh: m_meshChunks | std::views::values)
		{
			if (mesh == nullptr || mesh->get_chunk_mesh() == nullptr)
			{
				continue;
			}

			meshes++;
		}

		return meshes;
	}
	
	void ChunkBuilder::update(const glm::vec3 playerPosition)
	{
		const auto playerChunkPosition = world_pos_to_chunk(playerPosition);
		bool shouldGenerateData = playerChunkPosition != m_oldPlayerChunkPosition;

		// update deletion data
		m_threadPool.enqueue([this, playerChunkPosition, shouldGenerateData]
		{
			update_data_deletion_queue(playerChunkPosition);

			if (shouldGenerateData)
			{
				update_mesh_deletion_queue(playerChunkPosition);
			}
		});

		// update render data
		m_threadPool.enqueue(
		[this, playerChunkPosition, shouldGenerateData]
		{
			if (shouldGenerateData)
			{
				update_mesh_generation_queue(playerChunkPosition);
			}
		});

		// generate render data
		m_threadPool.enqueue([this, playerChunkPosition]
		{
			for (int i = 0; i < MAX_CHUNKS_DATA_PER_FRAME_GENERATED; i++)
			{
				auto lock = std::unique_lock(m_worldMutex);

				if (m_dataGenerationQueue.empty())
				{
					break;
				}

				const auto position = m_dataGenerationQueue.front();
				m_dataChunks[position]->generate_data(position);

				m_dataGenerationQueue.pop();
			}

			const int renderDistance = m_specs.RenderDistance;
			for (int i = 0; i < MAX_CHUNKS_PER_FRAME_GENERATED; i++)
			{
				auto lock = std::unique_lock(m_worldMutex);

				if (m_meshGenerationQueue.empty())
				{
					break;
				}

				const auto position = m_meshGenerationQueue.front();
				const auto xDistance = abs(position.X - playerChunkPosition.X);
				const auto yDistance = abs(position.Y - playerChunkPosition.Y);
				const auto zDistance = abs(position.Z - playerChunkPosition.Z);

				if (xDistance > renderDistance || yDistance > renderDistance || zDistance > renderDistance)
				{
					m_meshGenerationQueue.pop();
					continue;
				}

				const auto left = ChunkPosition(position.X - 1, position.Y, position.Z);
				if (m_dataChunks.contains(left) == false || m_dataChunks[left]->is_processed() == false)
				{
					break;
				}

				const auto down = ChunkPosition(position.X, position.Y - 1, position.Z);
				if (m_dataChunks.contains(down) == false || m_dataChunks[down]->is_processed() == false)
				{
					break;
				}

				const auto back = ChunkPosition(position.X, position.Y, position.Z - 1);
				if (m_dataChunks.contains(back) == false || m_dataChunks[back]->is_processed() == false)
				{
					break;
				}

				const auto right = ChunkPosition(position.X + 1, position.Y, position.Z);
				if (m_dataChunks.contains(right) == false || m_dataChunks[right]->is_processed() == false)
				{
					break;
				}

				const auto up = ChunkPosition(position.X, position.Y + 1, position.Z);
				if (m_dataChunks.contains(up) == false || m_dataChunks[up]->is_processed() == false)
				{
					break;
				}

				const auto front = ChunkPosition(position.X, position.Y, position.Z + 1);
				if (m_dataChunks.contains(front) == false || m_dataChunks[front]->is_processed() == false)
				{
					break;
				}

				generate_chunk_mesh(position);

				m_meshGenerationQueue.pop();
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

	void ChunkBuilder::enqueue_data_generation(ChunkPosition position)
	{
		if (m_dataChunks.contains(position))
		{
			return;
		}

		const auto chunk = std::make_shared<Chunk>(m_specs.ChunkSize * m_specs.ChunkSize * m_specs.ChunkSize);
		m_dataChunks.emplace(position, chunk);

		m_dataGenerationQueue.emplace(position);
	}

	void ChunkBuilder::update_mesh_generation_queue(const ChunkPosition playerChunkPosition)
	{
		const int renderDistance = m_specs.RenderDistance;

		for (int z = -renderDistance + playerChunkPosition.Z; z < renderDistance + playerChunkPosition.Z; ++z)
		{
			for (int y = -renderDistance + playerChunkPosition.Y; y < renderDistance + playerChunkPosition.Y; ++y)
			{
				for (int x = -renderDistance + playerChunkPosition.X; x < renderDistance + playerChunkPosition.X; ++x)
				{
					auto lock = std::unique_lock(m_worldMutex);

					auto chunkPosition = ChunkPosition(x, y, z);

					if (m_meshChunks.contains(chunkPosition))
					{
						continue;
					}

					enqueue_data_generation(chunkPosition);
					enqueue_data_generation(ChunkPosition(x + 1, y, z));
					enqueue_data_generation(ChunkPosition(x - 1, y, z));
					enqueue_data_generation(ChunkPosition(x, y + 1, z));
					enqueue_data_generation(ChunkPosition(x, y - 1, z));
					enqueue_data_generation(ChunkPosition(x, y, z + 1));
					enqueue_data_generation(ChunkPosition(x, y, z - 1));

					m_meshChunks.emplace(chunkPosition, nullptr);
					m_meshGenerationQueue.emplace(chunkPosition);
				}
			}
		}
	}

	void ChunkBuilder::update_render_queue(const ChunkPosition playerChunkPosition)
	{
		const int renderDistance = m_specs.RenderDistance;

		for (const auto& position: m_meshChunks | std::views::keys)
		{
			const auto xDistance = abs(position.X - playerChunkPosition.X);
			const auto yDistance = abs(position.Y - playerChunkPosition.Y);
			const auto zDistance = abs(position.Z - playerChunkPosition.Z);

			if (xDistance < renderDistance && yDistance < renderDistance && zDistance < renderDistance)
			{
				if (m_meshChunks[position] == nullptr || m_meshChunks[position]->get_chunk_mesh() == nullptr)
				{
					continue;
				}

				m_renderQueue.emplace(position, m_meshChunks[position]);
			}
		}
	}


	void ChunkBuilder::update_data_deletion_queue(const ChunkPosition playerChunkPosition)
	{
		auto lock = std::unique_lock(m_worldMutex);

		const int renderDistance = m_specs.RenderDistance;
		auto chunksToErase = std::vector<ChunkPosition>();
		for (const auto& position: m_dataChunks | std::views::keys)
		{
			const auto right = ChunkPosition(position.X + 1, position.Y, position.Z);
			const auto up = ChunkPosition(position.X, position.Y + 1, position.Z);
			const auto front = ChunkPosition(position.X, position.Y, position.Z + 1);
			const auto left = ChunkPosition(position.X - 1, position.Y, position.Z);
			const auto down = ChunkPosition(position.X, position.Y - 1, position.Z);
			const auto back = ChunkPosition(position.X, position.Y, position.Z - 1);

			if (m_meshChunks.contains(right) && m_meshChunks[right] != nullptr 
				&& m_meshChunks.contains(up) && m_meshChunks[up] != nullptr 
				&& m_meshChunks.contains(front) && m_meshChunks[front] != nullptr 
				&& m_meshChunks.contains(left) && m_meshChunks[left] != nullptr 
				&& m_meshChunks.contains(down) && m_meshChunks[down] != nullptr 
				&& m_meshChunks.contains(back) && m_meshChunks[back] != nullptr)
			{
				chunksToErase.emplace_back(position);

				continue;
			}

			const auto xDistance = abs(position.X - playerChunkPosition.X);
			const auto yDistance = abs(position.Y - playerChunkPosition.Y);
			const auto zDistance = abs(position.Z - playerChunkPosition.Z);

			if (xDistance > renderDistance * 2 || yDistance > renderDistance * 2 || zDistance > renderDistance * 2)
			{
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
		for (const auto& position: m_meshChunks | std::views::keys)
		{
			const auto xDistance = abs(position.X - playerChunkPosition.X);
			const auto yDistance = abs(position.Y - playerChunkPosition.Y);
			const auto zDistance = abs(position.Z - playerChunkPosition.Z);

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
				VulkanRenderer::queue_vao_free(m_meshChunks[position]->get_chunk_mesh());
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
			position.X += x < 0 ? -1 : 1;
		}

		if (y < 0 || y >= chunkSize)
		{
			actualY = y < 0 ? chunkSize - 1 : 0;
			position.Y += y < 0 ? -1 : 1;
		}

		if (z < 0 || z >= chunkSize)
		{
			actualZ = z < 0 ? chunkSize - 1 : 0;
			position.Z += z < 0 ? -1 : 1;
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
						auto quadDown = RenderQuad(Side::DOWN, positionOffset);
						quadDown.add_indices_offset(indexOffset + indexQuadOffset);
						totalVertices.insert(totalVertices.end(), quadDown.get_vertices().begin(), quadDown.get_vertices().end());
						totalIndices.insert(totalIndices.end(), quadDown.get_indices().begin(), quadDown.get_indices().end());

						indexQuadOffset += 4;
					}

					if (upBlock == false)
					{
						auto quadUp = RenderQuad(Side::UP, positionOffset);
						quadUp.add_indices_offset(indexOffset + indexQuadOffset);
						totalVertices.insert(totalVertices.end(), quadUp.get_vertices().begin(), quadUp.get_vertices().end());
						totalIndices.insert(totalIndices.end(), quadUp.get_indices().begin(), quadUp.get_indices().end());

						indexQuadOffset += 4;
					}

					if (leftBlock == false)
					{
						auto quadLeft = RenderQuad(Side::LEFT, positionOffset);
						quadLeft.add_indices_offset(indexOffset + indexQuadOffset);
						totalVertices.insert(totalVertices.end(), quadLeft.get_vertices().begin(), quadLeft.get_vertices().end());
						totalIndices.insert(totalIndices.end(), quadLeft.get_indices().begin(), quadLeft.get_indices().end());

						indexQuadOffset += 4;
					}

					if (rightBlock == false)
					{
						auto quadRight = RenderQuad(Side::RIGHT, positionOffset);
						quadRight.add_indices_offset(indexOffset + indexQuadOffset);
						totalVertices.insert(totalVertices.end(), quadRight.get_vertices().begin(), quadRight.get_vertices().end());
						totalIndices.insert(totalIndices.end(), quadRight.get_indices().begin(), quadRight.get_indices().end());

						indexQuadOffset += 4;
					}

					if (backBlock == false)
					{
						auto quadBack = RenderQuad(Side::BACK, positionOffset);
						quadBack.add_indices_offset(indexOffset + indexQuadOffset);
						totalVertices.insert(totalVertices.end(), quadBack.get_vertices().begin(), quadBack.get_vertices().end());
						totalIndices.insert(totalIndices.end(), quadBack.get_indices().begin(), quadBack.get_indices().end());

						indexQuadOffset += 4;
					}

					if (frontBlock == false)
					{
						auto quadFront = RenderQuad(Side::FRONT, positionOffset);
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
