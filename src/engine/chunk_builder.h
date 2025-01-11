#pragma once

#include "voxel_chunk.h"

#include <glm/glm.hpp>
#include <queue>

namespace Moxel
{
	struct ChunkWorldSpecs
	{
		int RenderDistance = 5;
	};

	class ChunkBuilder
	{
	public:
		ChunkBuilder(ChunkWorldSpecs specs = ChunkWorldSpecs());

		glm::vec3 ChunkPosToWorld(glm::vec3 chunkPosition) const;
		glm::vec3 WorldPosToChunk(glm::vec3 worldPosition) const;

		void Update(glm::vec3 playerPosition);
		void DestroyWorld();

		std::vector<std::shared_ptr<VoxelChunk>>& GetRenderChunks() { return m_renderChunks; }

	private:
		void UpdateDeletionQueue(glm::vec3 playerChunkPosition);
		void UpdateRenderQueue(glm::vec3 playerChunkPosition);

		struct Vec3Utils
		{
			size_t operator()(const glm::vec3& a) const 
			{
				return std::hash<float>{}(a.x + a.y + a.z); 
			}

			bool operator()(const glm::vec3& a, const glm::vec3& b) const
			{
				return a.x == b.x && a.y == b.y && a.z == b.z;
			}
		};

		const int MAX_CHUNKS_PER_FRAME_GENERATED = 16;
		const int MAX_CHUNKS_PER_FRAME_UNLOADED = 8;
		const int CHUNK_SIZE = 16;

		ChunkWorldSpecs m_specs;

		std::unordered_map<glm::vec3, std::shared_ptr<VoxelChunk>, Vec3Utils, Vec3Utils> m_totalChunks;
		std::vector<std::shared_ptr<VoxelChunk>> m_renderChunks;

		std::queue<std::shared_ptr<VoxelChunk>> m_renderQueue;
		std::queue<std::shared_ptr<VoxelChunk>> m_deletionQueue;
	};
}