#include "chunk_generator.h"
#include "render_quad.h"
#include "renderer/vulkan/vulkan_renderer.h"

#include <algorithm>
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
			if (chunk->get_chunk_mesh() == nullptr)
			{
				continue;
			}

			VulkanRenderer::queue_resource_free(chunk->get_chunk_mesh());
		}

		while (m_meshDeletionQueue.empty() == false)
		{
			VulkanRenderer::queue_resource_free(m_meshDeletionQueue.front().second->get_chunk_mesh());

			m_meshDeletionQueue.pop();
		}

		m_renderChunks.clear();
		m_meshChunks.clear();
	}
	
	void ChunkBuilder::update(const glm::vec3 playerPosition)
	{
		const auto playerChunkPosition = world_pos_to_chunk(playerPosition);

		// update deletion data
		update_deletion_queue(playerChunkPosition);
		for (int i = 0; i < MAX_CHUNKS_PER_FRAME_UNLOADED; i++)
		{
			if (m_meshDeletionQueue.empty() == true)
			{
				break;
			}

			const auto& [position, chunk] = m_meshDeletionQueue.front();
			if (chunk->get_chunk_mesh() != nullptr)
			{
				VulkanRenderer::queue_resource_free(chunk->get_chunk_mesh());
			}

			m_meshDeletionQueue.pop();
		}

		// update render data
		update_render_queue(playerChunkPosition);
		for (int i = 0; i < MAX_CHUNKS_PER_FRAME_GENERATED; i++)
		{
			if (m_meshGenerationQueue.empty() == true)
			{
				break;
			}

			generate_chunk_mesh(m_meshGenerationQueue.front().first);

			m_meshGenerationQueue.pop();
		}
	}

	void ChunkBuilder::update_render_queue(const glm::i32vec3 playerChunkPosition)
	{
		const int renderDistance = m_specs.RenderDistance;
		for (int z = -renderDistance + playerChunkPosition.z; z < renderDistance + playerChunkPosition.z; ++z)
		{
			for (int y = -renderDistance + playerChunkPosition.y; y < renderDistance + playerChunkPosition.y; ++y)
			{
				for (int x = -renderDistance + playerChunkPosition.x; x < renderDistance + playerChunkPosition.x; ++x)
				{
					auto chunkPosition = ChunkPosition(x, y, z);

					if (m_dataChunks.contains(chunkPosition) == false)
					{
						const auto chunk = std::make_shared<Chunk>(m_specs.ChunkSize * m_specs.ChunkSize * m_specs.ChunkSize);

						m_dataChunks.emplace(chunkPosition, chunk);
						m_dataChunks.at(chunkPosition)->generate_data(chunkPosition);

						continue;
					}

					if (m_meshChunks.contains(chunkPosition) == false && m_dataChunks.contains(chunkPosition) == true)
					{
						const auto left = ChunkPosition(chunkPosition.x - 1, chunkPosition.y, chunkPosition.z);
						if (m_dataChunks.contains(left) == false)
						{
							m_dataChunks.emplace(left, std::make_shared<Chunk>(m_specs.ChunkSize * m_specs.ChunkSize *
																			   m_specs.ChunkSize));
							m_dataChunks.at(left)->generate_data(left);
						}

						const auto down = ChunkPosition(chunkPosition.x, chunkPosition.y - 1, chunkPosition.z);
						if (m_dataChunks.contains(down) == false)
						{
							m_dataChunks.emplace(down, std::make_shared<Chunk>(m_specs.ChunkSize * m_specs.ChunkSize *
																			   m_specs.ChunkSize));
							m_dataChunks.at(down)->generate_data(down);
						}

						const auto back = ChunkPosition(chunkPosition.x, chunkPosition.y, chunkPosition.z - 1);
						if (m_dataChunks.contains(back) == false)
						{
							m_dataChunks.emplace(back, std::make_shared<Chunk>(m_specs.ChunkSize * m_specs.ChunkSize *
																			   m_specs.ChunkSize));
							m_dataChunks.at(back)->generate_data(back);
						}

						m_meshChunks[chunkPosition] = std::make_shared<ChunkMesh>(nullptr);
						m_meshGenerationQueue.emplace(chunkPosition, nullptr);
						continue;
					}

					if (m_meshChunks.contains(chunkPosition) == false || m_meshChunks.at(chunkPosition)->get_chunk_mesh() == nullptr)
					{
						continue;
					}

					m_renderChunks.emplace(chunkPosition, m_meshChunks.at(chunkPosition));
				}
			}
		}
	}

	void ChunkBuilder::update_deletion_queue(const glm::i32vec3 playerChunkPosition)
	{
		const auto renderDistance = m_specs.RenderDistance;
		auto chunksToErase = std::vector<ChunkPosition>();
		for (const auto& chunk: m_meshChunks)
		{
			const auto xDistance = abs(chunk.first.x - playerChunkPosition.x);
			const auto yDistance = abs(chunk.first.y - playerChunkPosition.y);
			const auto zDistance = abs(chunk.first.z - playerChunkPosition.z);

			if (xDistance > renderDistance || yDistance > renderDistance || zDistance > renderDistance)
			{
				m_meshDeletionQueue.emplace(chunk);
				chunksToErase.emplace_back(chunk.first);
			}
		}

		for (const auto& position: chunksToErase)
		{
			m_meshChunks.erase(position);
		}
	}

	glm::vec3 ChunkBuilder::chunk_to_world_pos(const glm::vec3 chunkPosition) const
	{ 
		return chunkPosition * static_cast<float>(m_specs.ChunkSize);
	}

	glm::vec3 ChunkBuilder::world_pos_to_chunk(const glm::vec3 worldPosition) const
	{
		const auto chunkSize = static_cast<float>(m_specs.ChunkSize);

		return {
			static_cast<int>(worldPosition.x / chunkSize),
			static_cast<int>(worldPosition.y / chunkSize),
			static_cast<int>(worldPosition.z / chunkSize)
		};
	}

	int ChunkBuilder::get_voxel_index(const glm::i32vec3 position) const
	{
		return position.z | (position.y << m_specs.ChunkBitSize) | (position.x << m_specs.ChunkBitSize * 2);
	}

	bool ChunkBuilder::get_voxel(ChunkPosition position, int x, int y, int z) const
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
		auto totalVertices = std::vector<Vertex>();
		auto totalIndices = std::vector<uint32_t>();
		int indexOffset = 0;

		const int chunkSize = m_specs.ChunkSize;
		for (int z = 0; z < chunkSize; ++z)
		{
			for (int y = 0; y < chunkSize; ++y)
			{
				for (int x = 0; x < chunkSize; ++x)
				{
					const auto positionOffset = glm::i32vec3(x, y, z);
					int indexQuadOffset = 0;

					const auto mainBlock = m_dataChunks.at(position)->get_block(z * chunkSize * chunkSize + y * chunkSize + x);
					const auto leftBlock = get_voxel(position, x - 1, y, z);
					const auto downBlock = get_voxel(position, x, y - 1, z);
					const auto backBlock = get_voxel(position, x, y, z - 1);

					switch (mainBlock)
					{
						case true:
						{
							// down quad
							if (downBlock == false)
							{
								auto quadDown = RenderQuad(Side::Down, positionOffset);
								quadDown.add_indices_offset(indexOffset + indexQuadOffset);
								totalVertices.insert(totalVertices.end(), quadDown.get_vertices().begin(), quadDown.get_vertices().end());
								totalIndices.insert(totalIndices.end(), quadDown.get_indices().begin(), quadDown.get_indices().end());

								indexQuadOffset += 4;
							}

							// left quad
							if (leftBlock == false)
							{
								auto quadLeft = RenderQuad(Side::Left, positionOffset);
								quadLeft.add_indices_offset(indexOffset + indexQuadOffset);
								totalVertices.insert(totalVertices.end(), quadLeft.get_vertices().begin(), quadLeft.get_vertices().end());
								totalIndices.insert(totalIndices.end(), quadLeft.get_indices().begin(), quadLeft.get_indices().end());

								indexQuadOffset += 4;
							}

							// back quad
							if (backBlock == false)
							{
								auto quadBack = RenderQuad(Side::Back, positionOffset);
								quadBack.add_indices_offset(indexOffset + indexQuadOffset);
								totalVertices.insert(totalVertices.end(), quadBack.get_vertices().begin(), quadBack.get_vertices().end());
								totalIndices.insert(totalIndices.end(), quadBack.get_indices().begin(), quadBack.get_indices().end());

								indexQuadOffset += 4;
							}
							break;
						}
						case false:
						{
							// up quad
							if (downBlock == true)
							{
								auto quadUp = RenderQuad(Side::Down, positionOffset);
								quadUp.add_indices_offset(indexOffset + indexQuadOffset);
								totalVertices.insert(totalVertices.end(), quadUp.get_vertices().begin(), quadUp.get_vertices().end());
								totalIndices.insert(totalIndices.end(), quadUp.get_indices().begin(), quadUp.get_indices().end());

								indexQuadOffset += 4;
							}

							// right quad
							if (leftBlock == true)
							{
								auto quadRight = RenderQuad(Side::Left, positionOffset);
								quadRight.add_indices_offset(indexOffset + indexQuadOffset);
								totalVertices.insert(totalVertices.end(), quadRight.get_vertices().begin(), quadRight.get_vertices().end());
								totalIndices.insert(totalIndices.end(), quadRight.get_indices().begin(), quadRight.get_indices().end());

								indexQuadOffset += 4;
							}

							// front quad
							if (backBlock == true)
							{
								auto quadFront = RenderQuad(Side::Back, positionOffset);
								quadFront.add_indices_offset(indexOffset + indexQuadOffset);
								totalVertices.insert(totalVertices.end(), quadFront.get_vertices().begin(), quadFront.get_vertices().end());
								totalIndices.insert(totalIndices.end(), quadFront.get_indices().begin(), quadFront.get_indices().end());

								indexQuadOffset += 4;
							}
							break;
						}
					}

					indexOffset += indexQuadOffset;
				}
			}
		}

		if (totalIndices.empty())
		{
			return;
		}

		const auto vao = std::make_shared<VulkanVertexArray>(totalIndices, totalVertices);
		m_meshChunks[position] = std::make_shared<ChunkMesh>(vao);
	}
}