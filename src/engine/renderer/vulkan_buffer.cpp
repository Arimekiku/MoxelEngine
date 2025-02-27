#include "vulkan_buffer.h"
#include "vulkan_renderer.h"
#include "engine/application.h"

namespace Moxel
{
	//
	// VulkanVertexArray
	//

	VulkanVertexArray::VulkanVertexArray(const std::vector<uint32_t>& indices, const std::vector<VoxelVertex>& vertices)
	{
		auto& allocator = Application::get().get_allocator();

		// vertex buffer
		m_vertices = vertices;
		const auto verticesSize = vertices.size() * sizeof(vertices[0]);
		auto vertexBufferInfo = VkBufferCreateInfo();
		vertexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		vertexBufferInfo.pNext = nullptr;
		vertexBufferInfo.size = verticesSize;
		vertexBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

		constexpr auto vertexMemoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;

		m_vertexBuffer = allocator.allocate_buffer(vertexBufferInfo, vertexMemoryUsage);

		// index buffer
		m_indices = indices.size();
		const auto indicesSize = indices.size() * sizeof(indices[0]);
		auto indexBufferInfo = VkBufferCreateInfo();
		indexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		indexBufferInfo.pNext = nullptr;
		indexBufferInfo.size = indicesSize;
		indexBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

		constexpr auto indexMemoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;

		m_indexBuffer = allocator.allocate_buffer(indexBufferInfo, indexMemoryUsage);

		// allocate vertex array data
		auto stagingBufferInfo = VkBufferCreateInfo();
		stagingBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		stagingBufferInfo.pNext = nullptr;
		stagingBufferInfo.size = verticesSize + indicesSize;
		stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

		constexpr auto stagingMemoryUsage = VMA_MEMORY_USAGE_CPU_ONLY;

		const auto stagingBuffer = allocator.allocate_buffer(stagingBufferInfo, stagingMemoryUsage);

		// copy buffers
		void* data = stagingBuffer.AllocationInfo.pMappedData;
		memcpy(data, vertices.data(), verticesSize);
		memcpy(static_cast<char*>(data) + verticesSize, indices.data(), indicesSize);

		VulkanRenderer::immediate_submit([&](const VkCommandBuffer cmd)
		{
			auto vertexCopy = VkBufferCopy();
			vertexCopy.dstOffset = 0;
			vertexCopy.srcOffset = 0;
			vertexCopy.size = verticesSize;
			vkCmdCopyBuffer(cmd, stagingBuffer.Buffer, m_vertexBuffer.Buffer, 1, &vertexCopy);

			auto indexCopy = VkBufferCopy();
			indexCopy.dstOffset = 0;
			indexCopy.srcOffset = verticesSize;
			indexCopy.size = indicesSize;
			vkCmdCopyBuffer(cmd, stagingBuffer.Buffer, m_indexBuffer.Buffer, 1, &indexCopy);
		});

		allocator.destroy_buffer(stagingBuffer);
	}

	VulkanVertexArray::~VulkanVertexArray()
	{
		VulkanRenderer::free_resource_submit([vertex = m_vertexBuffer, index = m_indexBuffer]()
		{
			auto allocator = Application::get().get_allocator();

			allocator.destroy_buffer(vertex);
			allocator.destroy_buffer(index);
		});
	}

	//
	// VulkanBufferUniform
	//

	VulkanBufferUniform::VulkanBufferUniform(const uint32_t bufferSize)
	{
		m_size = bufferSize;

		auto bufferInfo = VkBufferCreateInfo();
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		bufferInfo.size = m_size;

		m_buffer = Application::get().get_allocator().allocate_buffer(bufferInfo, VMA_MEMORY_USAGE_CPU_TO_GPU);

		m_descriptorInfo.buffer = m_buffer.Buffer;
		m_descriptorInfo.offset = 0;
		m_descriptorInfo.range = m_size;
	}

	VulkanBufferUniform::~VulkanBufferUniform()
	{
		VulkanRenderer::free_resource_submit([buffer = m_buffer]()
		{
			auto allocator = Application::get().get_allocator();

			allocator.destroy_buffer(buffer);
		});
	}

	void VulkanBufferUniform::write_data(const void* data, const uint32_t size) const
	{
		memcpy(m_buffer.AllocationInfo.pMappedData, data, size);
	}
}