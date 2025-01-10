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
		~VulkanVertexArray();
		
		bool IsEmpty() const { return m_Indices == 0 || m_Vertices.size() == 0; }
		void Destroy();

		const std::vector<Vertex>& GetVertices() { return m_Vertices; }
		size_t GetIndexBufferSize() const { return m_Indices; }

		VulkanBuffer GetVertexBuffer() const { return m_VertexBuffer; }
		VulkanBuffer GetIndexBuffer() const { return m_IndexBuffer; }

	private:
		std::vector<Vertex> m_Vertices;
		size_t m_Indices = 0;

		VulkanBuffer m_VertexBuffer;
		VulkanBuffer m_IndexBuffer;
	};
}
