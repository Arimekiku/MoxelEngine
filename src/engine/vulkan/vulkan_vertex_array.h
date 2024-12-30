#pragma once

#include <span>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>
#include <glm/glm.hpp>

namespace SDLarria
{
	struct GPUDrawPushConstants_TEST
	{
		glm::mat4 worldMatrix;
		VkDeviceAddress vertexBuffer;
	};

	struct Vertex
	{
		glm::vec3 position;
		float uv_x;
		glm::vec3 normal;
		float uv_y;
		glm::vec4 color;
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

		VkDeviceAddress GetRenderingID() const { return m_VertexBufferAddress; }
		BufferArray GetVertexBuffer() const { return m_VertexBuffer; }
		BufferArray GetIndexBuffer() const { return m_IndexBuffer; }

	private:
		VkDeviceAddress m_VertexBufferAddress = 0;
		BufferArray m_VertexBuffer;
		BufferArray m_IndexBuffer;
	};
}
