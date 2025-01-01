#include "vulkan_vertex_array.h"
#include "vulkan.h"
#include "vulkan_allocator.h"

namespace SDLarria
{
	VulkanVertexArray::VulkanVertexArray(const std::span<uint32_t> indices, const std::span<Vertex> vertices)
	{
		// vertex buffer
		const auto verticesSize = vertices.size() * sizeof(vertices[0]);
		auto vertexBufferInfo = VkBufferCreateInfo();
		vertexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		vertexBufferInfo.pNext = nullptr;
		vertexBufferInfo.size = verticesSize;
		vertexBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

		constexpr auto vertexMemoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;

		m_VertexBuffer = VulkanAllocator::AllocateBuffer(vertexBufferInfo, vertexMemoryUsage);

		// index buffer
		const auto indicesSize = indices.size() * sizeof(indices[0]);
		auto indexBufferInfo = VkBufferCreateInfo();
		indexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		indexBufferInfo.pNext = nullptr;
		indexBufferInfo.size = indicesSize;
		indexBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

		constexpr auto indexMemoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;

		m_IndexBuffer = VulkanAllocator::AllocateBuffer(indexBufferInfo, indexMemoryUsage);

		// allocate vertex array data
		auto stagingBufferInfo = VkBufferCreateInfo();
		stagingBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		stagingBufferInfo.pNext = nullptr;
		stagingBufferInfo.size = verticesSize + indicesSize;
		stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

		constexpr auto stagingMemoryUsage = VMA_MEMORY_USAGE_CPU_ONLY;

		const auto stagingBuffer = VulkanAllocator::AllocateBuffer(stagingBufferInfo, stagingMemoryUsage);

		// copy buffers
		void* data = stagingBuffer.AllocationInfo.pMappedData;
		memcpy(data, vertices.data(), verticesSize);
		memcpy((char*)data + verticesSize, indices.data(), indicesSize);

		VulkanRenderer::Get().ImmediateSubmit([&](VkCommandBuffer cmd)
		{
			auto vertexCopy = VkBufferCopy();
			vertexCopy.dstOffset = 0;
			vertexCopy.srcOffset = 0;
			vertexCopy.size = verticesSize;
			vkCmdCopyBuffer(cmd, stagingBuffer.Buffer, m_VertexBuffer.Buffer, 1, &vertexCopy);

			auto indexCopy = VkBufferCopy();
			indexCopy.dstOffset = 0;
			indexCopy.srcOffset = verticesSize;
			indexCopy.size = indicesSize;
			vkCmdCopyBuffer(cmd, stagingBuffer.Buffer, m_IndexBuffer.Buffer, 1, &indexCopy);
		});

		VulkanAllocator::DestroyBuffer(stagingBuffer);
	}
}
