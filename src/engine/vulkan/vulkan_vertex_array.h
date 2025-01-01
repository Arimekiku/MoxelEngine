#pragma once

#include "vulkan_buffer.h"

#include <span>
#include <glm/glm.hpp>

namespace SDLarria
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
		VulkanVertexArray(std::span<uint32_t> indices, std::span<Vertex> vertices);

		const std::vector<Vertex>& GetVertices() { return m_Vertices; }

		VulkanBuffer GetVertexBuffer() const { return m_VertexBuffer; }
		VulkanBuffer GetIndexBuffer() const { return m_IndexBuffer; }

	private:
		std::vector<Vertex> m_Vertices;

		VulkanBuffer m_VertexBuffer;
		VulkanBuffer m_IndexBuffer;
	};
}
