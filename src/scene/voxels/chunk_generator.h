#pragma once

#include "chunk.h"
#include "render_quad.h"
#include "engine/core/thread_pool.h"

#include <glm/glm.hpp>
#include <queue>
#include <memory>
#include <mutex>

namespace Moxel
{
	struct ChunkWorldSpecs
	{
		int ChunkSize = 16;
		int ChunkBitSize = 4; // 2^4 = 16

		int RenderDistance = 5;
	};

	class ChunkBuilder
	{
	public:
		ChunkBuilder(ChunkWorldSpecs specs = ChunkWorldSpecs());

		glm::vec3 chunk_to_world_pos(glm::vec3 chunkPosition) const;
		ChunkPosition world_pos_to_chunk(glm::vec3 worldPosition) const;

		void update(glm::vec3 playerPosition);
		void destroy_world();

		int get_total_chunks_data_count() const;
		int get_total_chunks_mesh_count();

		std::queue<std::pair<ChunkPosition, std::shared_ptr<ChunkMesh>>>& get_render_queue() { return m_renderQueue; }
	private:
		void generate_chunk_mesh(ChunkPosition position);
		bool get_voxel(ChunkPosition position, int x, int y, int z) const;

		void update_mesh_generation_queue(ChunkPosition playerChunkPosition);
		void update_mesh_deletion_queue(ChunkPosition playerChunkPosition);

		void enqueue_data_generation(ChunkPosition position);
		void update_data_deletion_queue(ChunkPosition playerChunkPosition);

		void update_render_queue(ChunkPosition playerChunkPosition);

		const int MAX_CHUNKS_PER_FRAME_GENERATED = 16;
		const int MAX_CHUNKS_DATA_PER_FRAME_GENERATED = 32;

		ChunkWorldSpecs m_specs;
		ChunkPosition m_oldPlayerChunkPosition = {100, 100, 100};

		std::unordered_map<ChunkPosition, std::shared_ptr<Chunk>> m_dataChunks;
		std::unordered_map<ChunkPosition, std::shared_ptr<ChunkMesh>> m_meshChunks;

		std::unordered_map<ChunkPosition, std::pair<std::vector<uint32_t>, std::vector<VoxelVertex>>> m_requestedMeshes;

		std::queue<ChunkPosition> m_dataGenerationQueue;
		std::queue<ChunkPosition> m_meshGenerationQueue;
		std::queue<std::pair<ChunkPosition, std::shared_ptr<ChunkMesh>>> m_renderQueue;

		std::mutex m_worldMutex;
		ThreadPool m_threadPool;
	};
}
