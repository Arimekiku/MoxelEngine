#include "chunk_builder.h"

#include <algorithm>

namespace Moxel
{
	ChunkBuilder::ChunkBuilder(const ChunkWorldSpecs specs)
	{ 
		m_specs = specs;

		const int renderDistance = m_specs.RenderDistance;
		for (int z = -renderDistance; z < renderDistance; ++z)
		{
			for (int y = -renderDistance; y < renderDistance; ++y)
			{
				for (int x = -renderDistance; x < renderDistance; ++x)
				{
					const glm::vec3 chunkPosition = glm::vec3(x, y, z);

					m_totalChunks.emplace(chunkPosition, std::make_shared<VoxelChunk>(chunkPosition));
				}
			}
		}
	}

	void ChunkBuilder::DestroyWorld() 
	{ 
		m_renderChunks.clear();
		m_totalChunks.clear();
	}
	
	void ChunkBuilder::Update(const glm::vec3 playerPosition) 
	{
		const auto playerChunkPosition = WorldPosToChunk(playerPosition);

		// update deletion data
		UpdateDeletionQueue(playerChunkPosition);
		for (int i = 0; i < MAX_CHUNKS_PER_FRAME_UNLOADED;)
		{
			if (m_deletionQueue.empty() == true)
			{
				break;
			}

			auto& chunk = m_deletionQueue.front();

			if (chunk->IsEmpty())
			{
				m_totalChunks.erase(chunk->GetPosition());
				m_deletionQueue.pop();
				continue;
			}

			chunk->GetVertexArray()->GetVertexArray()->Destroy();
			m_deletionQueue.pop();
			i++;
		}

		// update render data
		UpdateRenderQueue(playerChunkPosition);
		std::vector<std::shared_ptr<VoxelChunk>> chunksToParse;
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
		[this](const std::shared_ptr<VoxelChunk>& chunk)
		{
			chunk->RecreateChunk();
			m_renderChunks.push_back(chunk);
		});
	}

	void ChunkBuilder::UpdateRenderQueue(const glm::vec3 playerChunkPosition) 
	{
		const int renderDistance = m_specs.RenderDistance;
		for (int z = -renderDistance; z < renderDistance; ++z)
		{
			for (int y = -renderDistance; y < renderDistance; ++y)
			{
				for (int x = -renderDistance; x < renderDistance; ++x)
				{
					glm::vec3 chunkPosition = glm::vec3(x, y, z) + playerChunkPosition;

					if (m_totalChunks.contains(chunkPosition) == false)
					{
						m_totalChunks.emplace(chunkPosition, std::make_shared<VoxelChunk>(chunkPosition));
						m_renderQueue.push(m_totalChunks[chunkPosition]);
						continue;
					}

					m_renderChunks.push_back(m_totalChunks[chunkPosition]);
				}
			}
		}
	}

	void ChunkBuilder::UpdateDeletionQueue(const glm::vec3 playerChunkPosition) 
	{
		const auto renderDistance = m_specs.RenderDistance;
		for (const auto& [position, chunk]: m_totalChunks)
		{
			// TODO: fix IsEmpty() situation
			if (chunk->IsSetForDeletion())
			{
				if (chunk->IsEmpty())
				{
					m_deletionQueue.push(chunk);
				}

				continue;
			}

			const auto xDistance = abs(position.x - playerChunkPosition.x);
			const auto yDistance = abs(position.y - playerChunkPosition.y);
			const auto zDistance = abs(position.z - playerChunkPosition.z);

			if (xDistance > renderDistance || yDistance > renderDistance || zDistance > renderDistance)
			{
				m_deletionQueue.push(chunk);
				chunk->SetForDeletion();
			}
		}
	}

	glm::vec3 ChunkBuilder::ChunkPosToWorld(const glm::vec3 chunkPosition) const 
	{ 
		return chunkPosition * (float)CHUNK_SIZE;
	}

	glm::vec3 ChunkBuilder::WorldPosToChunk(const glm::vec3 worldPosition) const 
	{
		return glm::vec3(
			(int) (worldPosition.x / CHUNK_SIZE), 
			(int) (worldPosition.y / CHUNK_SIZE),
			(int) (worldPosition.z / CHUNK_SIZE)
		);
	}
}