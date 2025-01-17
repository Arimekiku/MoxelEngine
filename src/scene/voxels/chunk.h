#pragma once

#include "engine/renderer/vulkan_buffer.h"

#include <vector>
#include <memory>

namespace Moxel
{
	struct ChunkPosition
	{
		int X, Y, Z;

		ChunkPosition(const int x, const int y, const int z)
			: X(x), Y(y), Z(z) { }

		bool operator==(const ChunkPosition& second) const
		{
			return X == second.X && Y == second.Y && Z == second.Z;
		}
	};

	class Chunk
	{
	public:
		Chunk(int size);
		~Chunk();

		bool get_block(const int index) const { return m_blocks[index]; }
		void set_block(const int index) { m_blocks[index] = true; }

		bool is_processed() const { return m_isProcessed; }

		void generate_data(ChunkPosition position);
	private:
		std::vector<bool> m_blocks;

		bool m_isProcessed = false;
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
		const size_t hx = std::hash<int>()(key.X);
		const size_t hy = std::hash<int>()(key.Y);
		const size_t hz = std::hash<int>()(key.Z);

		return hx ^	((hy << 1) >> 1) ^ (hz << 1);
	}
};