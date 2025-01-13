#pragma once

#include <vector>
#include <memory>

#include "renderer/vulkan/vulkan_buffer_vertex_array.h"

namespace Moxel
{
	struct ChunkPosition
	{
		int x, y, z;

		ChunkPosition(const int x, const int y, const int z)
			: x(x), y(y), z(z) { }

		bool operator==(const ChunkPosition& check) const { return x == check.x && y == check.y && z == check.z; }
	};

	class Chunk
	{
	public:
		Chunk(int size);
		~Chunk();

		bool get_block(const int index) const { return m_blocks[index]; }
		void set_block(const int index) { m_blocks[index] = true; }

		bool is_processed() const { return m_is_processed; }

		void generate_data(ChunkPosition position);
	private:
		std::vector<bool> m_blocks;

		bool m_is_processed = false;
	};

	class ChunkMesh
	{
	public:
		ChunkMesh(const std::shared_ptr<VulkanVertexArray>& mesh)
			: m_chunkMesh(mesh) { }
		~ChunkMesh();

		void destroy_mesh();

		const std::shared_ptr<VulkanVertexArray>& get_chunk_mesh() { return m_chunkMesh; }
	private:
		std::shared_ptr<VulkanVertexArray> m_chunkMesh = nullptr;
	};
}

template<>
struct std::hash<Moxel::ChunkPosition>
{
	std::size_t operator()(const Moxel::ChunkPosition& key) const noexcept
	{
		return std::hash<int>()(key.x ^ key.y ^ key.z);
	}
};