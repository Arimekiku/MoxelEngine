#include "chunk_generator.h"
#include "render_quad.h"
#include "renderer/vulkan/vulkan_renderer.h"
#include "renderer/core/logger/log.h"

#include <PerlinNoise.hpp>
#include <algorithm>

namespace Moxel
{
	ChunkBuilder::ChunkBuilder(const ChunkWorldSpecs specs)
	{
		m_specs = specs;
	}

	void ChunkBuilder::destroy_world()
	{ 
		for (const auto& chunk: m_totalChunks)
		{
			if (chunk.second->get_chunk_mesh() == nullptr)
			{
				continue;
			}

			VulkanRenderer::QueueResourceFree(chunk.second->get_chunk_mesh());
		}

		while (m_deletionQueue.empty() == false)
		{
			VulkanRenderer::QueueResourceFree(m_deletionQueue.front().second->get_chunk_mesh());

			m_deletionQueue.pop();
		}

		m_renderChunks.clear();
		m_totalChunks.clear();
	}
	
	void ChunkBuilder::update(const glm::vec3 playerPosition)
	{
		const auto playerChunkPosition = world_pos_to_chunk(playerPosition);

		// update deletion data
		update_deletion_queue(playerChunkPosition);
		for (int i = 0; i < MAX_CHUNKS_PER_FRAME_UNLOADED; i++)
		{
			if (m_deletionQueue.empty() == true)
			{
				break;
			}

			const auto& chunk = m_deletionQueue.front();

			if (chunk.second->get_chunk_mesh() != nullptr)
			{
				VulkanRenderer::QueueResourceFree(chunk.second->get_chunk_mesh());
			}

			m_deletionQueue.pop();
		}

		// update render data
		update_render_queue(playerChunkPosition);
		std::vector<std::pair<ChunkPosition, std::shared_ptr<Chunk>>> chunksToParse;
		for (int i = 0; i < MAX_CHUNKS_PER_FRAME_GENERATED; i++)
		{
			if (m_renderQueue.empty() == true)
			{
				break;
			}

			chunksToParse.push_back(m_renderQueue.front());
			m_renderQueue.pop();
		}

		// use parallel for now
		std::ranges::for_each(chunksToParse,
		[this](const std::pair<ChunkPosition, std::shared_ptr<Chunk>>& chunk)
		{
			chunk.second->generate_mesh(chunk.first);
		});
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

					if (m_totalChunks.contains(chunkPosition) == false)
					{
						const auto chunk = std::make_shared<Chunk>(m_specs.ChunkSize * m_specs.ChunkSize * m_specs.ChunkSize);

						m_totalChunks.emplace(chunkPosition, chunk);
						m_renderQueue.emplace(chunkPosition, chunk);

						continue;
					}

					if (m_totalChunks.at(chunkPosition)->get_chunk_mesh() == nullptr)
					{
						continue;
					}

					m_renderChunks.emplace(chunkPosition, m_totalChunks[chunkPosition]);
				}
			}
		}
	}

	void ChunkBuilder::update_deletion_queue(const glm::i32vec3 playerChunkPosition)
	{
		const auto renderDistance = m_specs.RenderDistance;
		auto chunksToErase = std::vector<ChunkPosition>();
		for (const auto& chunk: m_totalChunks)
		{
			const auto xDistance = abs(chunk.first.x - playerChunkPosition.x);
			const auto yDistance = abs(chunk.first.y - playerChunkPosition.y);
			const auto zDistance = abs(chunk.first.z - playerChunkPosition.z);

			if (xDistance > renderDistance || yDistance > renderDistance || zDistance > renderDistance)
			{
				m_deletionQueue.emplace(chunk);
				chunksToErase.emplace_back(chunk.first);
			}
		}

		for (const auto& position: chunksToErase)
		{
			m_totalChunks.erase(position);
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
}