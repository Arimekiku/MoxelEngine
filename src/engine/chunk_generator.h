#pragma once

#include "chunk.h"

#include <glm/glm.hpp>
#include <queue>
#include <memory>

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
		glm::vec3 world_pos_to_chunk(glm::vec3 worldPosition) const;

		void update(glm::vec3 playerPosition);
		void destroy_world();

		std::unordered_map<ChunkPosition, std::shared_ptr<ChunkMesh>>& get_render_chunks() { return m_renderChunks; }
	private:
		void generate_chunk_mesh(ChunkPosition position);
		bool get_voxel(ChunkPosition position, int x, int y, int z) const;

		void update_mesh_generation_queue(glm::i32vec3 playerChunkPosition);
		void update_mesh_deletion_queue(glm::i32vec3 playerChunkPosition);

		void update_data_generation_queue(glm::i32vec3 playerChunkPosition);
		void update_data_deletion_queue();

		const int MAX_CHUNKS_PER_FRAME_GENERATED = 16;
		const int MAX_CHUNKS_DATA_PER_FRAME_GENERATED = 32;

		ChunkWorldSpecs m_specs;

		std::unordered_map<ChunkPosition, std::shared_ptr<Chunk>> m_dataChunks;
		std::queue<std::pair<ChunkPosition, std::shared_ptr<Chunk>>> m_dataGenerationQueue;

		std::unordered_map<ChunkPosition, std::shared_ptr<ChunkMesh>> m_meshChunks;
		std::queue<std::pair<ChunkPosition, std::shared_ptr<ChunkMesh>>> m_meshGenerationQueue;

		std::unordered_map<ChunkPosition, std::shared_ptr<ChunkMesh>> m_renderChunks;
	};
}