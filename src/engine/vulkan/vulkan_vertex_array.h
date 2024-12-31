#pragma once

#include <span>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>
#include <glm/glm.hpp>

namespace SDLarria
{
	struct Vertex
	{
		glm::vec3 Position;
		glm::vec3 Color;
	};

	struct BufferArray
	{
		VkBuffer Buffer = nullptr;
		VmaAllocation Allocation = nullptr;
		VmaAllocationInfo AllocationInfo = VmaAllocationInfo();
	};

	class VulkanVertexArray
	{
	public:
		VulkanVertexArray() = default;
		VulkanVertexArray(VmaAllocator allocator, std::span<uint32_t> indices, std::span<Vertex> vertices);

		const std::vector<Vertex>& GetVertices() { return m_Vertices; }

		BufferArray GetVertexBuffer() const { return m_VertexBuffer; }
		BufferArray GetIndexBuffer() const { return m_IndexBuffer; }

	private:
		std::vector<Vertex> m_Vertices;

		BufferArray m_VertexBuffer;
		BufferArray m_IndexBuffer;
	};
}
