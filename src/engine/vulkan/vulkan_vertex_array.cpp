#include "vulkan_vertex_array.h"
#include "vulkan.h"

namespace SDLarria
{
	static BufferArray CreateBufferArray(VmaAllocator allocator, const size_t bufferSize, const VkBufferUsageFlags bufferUsage, const VmaMemoryUsage memoryUsage)
	{
		auto buffer = BufferArray();

		auto vertexBufferInfo = VkBufferCreateInfo();
		vertexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		vertexBufferInfo.pNext = nullptr;
		vertexBufferInfo.size = bufferSize;
		vertexBufferInfo.usage =  bufferUsage;

		auto vertexAllocationInfo = VmaAllocationCreateInfo();
		vertexAllocationInfo.usage = memoryUsage;
		vertexAllocationInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

		const auto result = vmaCreateBuffer(allocator, &vertexBufferInfo, &vertexAllocationInfo, &buffer.Buffer, &buffer.Allocation, &buffer.AllocationInfo);
		VULKAN_CHECK(result);

		return buffer;
	}

	VulkanVertexArray::VulkanVertexArray(VmaAllocator allocator, const std::span<uint32_t> indices, const std::span<Vertex> vertices)
	{
		// vertex buffer
		const size_t vertexBufferSize = vertices.size() * sizeof(vertices[0]);
		constexpr auto vertexBufferUsage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		constexpr auto vertexMemoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;

		m_VertexBuffer = CreateBufferArray(allocator, vertexBufferSize, vertexBufferUsage, vertexMemoryUsage);

		// index buffer
		const size_t indexBufferSize = indices.size() * sizeof(indices[0]);
		constexpr auto indexBufferUsage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		constexpr auto indexMemoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;

		m_IndexBuffer = CreateBufferArray(allocator, indexBufferSize, indexBufferUsage, indexMemoryUsage);

		// allocate vertex array data
		auto [buffer, allocation, allocationInfo] = CreateBufferArray(allocator, vertexBufferSize + indexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
		void* data = allocationInfo.pMappedData;

		// copy buffers
		memcpy(data, vertices.data(), vertexBufferSize);
		memcpy((char*)data + vertexBufferSize, indices.data(), indexBufferSize);

		VulkanRenderer::Get().ImmediateSubmit([&](VkCommandBuffer cmd)
		{
			auto vertexCopy = VkBufferCopy();
			vertexCopy.dstOffset = 0;
			vertexCopy.srcOffset = 0;
			vertexCopy.size = vertexBufferSize;
			vkCmdCopyBuffer(cmd, buffer, m_VertexBuffer.Buffer, 1, &vertexCopy);

			auto indexCopy = VkBufferCopy();
			indexCopy.dstOffset = 0;
			indexCopy.srcOffset = vertexBufferSize;
			indexCopy.size = indexBufferSize;
			vkCmdCopyBuffer(cmd, buffer, m_IndexBuffer.Buffer, 1, &indexCopy);
		});

		vmaDestroyBuffer(allocator, buffer, allocation);
	}
}
