#pragma once

#include "vulkan_buffer.h"

#include <glm/glm.hpp>

namespace Moxel
{
	struct Vertex
	{
		glm::vec3 Position;
		glm::vec3 Color;
	};

	class VulkanVertexArray
	{
	public:
		VulkanVertexArray() = default;
		VulkanVertexArray(const std::vector<uint32_t>& indices, const std::vector<Vertex>& vertices);
		
		const std::vector<Vertex>& get_vertices() { return m_vertices; }
		size_t get_index_buffer_size() const { return m_indices; }

		VulkanBuffer& get_vertex_buffer() { return m_vertexBuffer; }
		VulkanBuffer& get_index_buffer() { return m_indexBuffer; }
	private:
		std::vector<Vertex> m_vertices;
		size_t m_indices = 0;

		VulkanBuffer m_vertexBuffer;
		VulkanBuffer m_indexBuffer;
	};
}
