#pragma once

#include "vulkan_buffer.h"

#include <glm/glm.hpp>

#include "engine/render_quad.h"

namespace Moxel
{
	struct Vertex
	{
		uint32_t Position;
		glm::vec3 Color;
	};

	class VulkanVertexArray
	{
	public:
		VulkanVertexArray() = default;
		VulkanVertexArray(const std::vector<uint32_t>& indices, const std::vector<VoxelVertex>& vertices);
		
		const std::vector<VoxelVertex>& get_vertices() { return m_vertices; }
		size_t get_index_buffer_size() const { return m_indices; }

		VulkanBuffer& get_vertex_buffer() { return m_vertexBuffer; }
		VulkanBuffer& get_index_buffer() { return m_indexBuffer; }
	private:
		std::vector<VoxelVertex> m_vertices;
		size_t m_indices = 0;

		VulkanBuffer m_vertexBuffer;
		VulkanBuffer m_indexBuffer;
	};
}
