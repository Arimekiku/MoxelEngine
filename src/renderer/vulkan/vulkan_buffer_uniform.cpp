#include "vulkan_buffer_uniform.h"
#include "vulkan_allocator.h"
#include "vulkan_renderer.h"

namespace Moxel
{
	struct GlobalRenderData;

	VulkanBufferUniform::VulkanBufferUniform(const uint32_t bufferSize)
	{
		m_Size = bufferSize;

		auto bufferInfo = VkBufferCreateInfo();
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		bufferInfo.size = m_Size;

		m_Buffer = VulkanAllocator::AllocateBuffer(bufferInfo, VMA_MEMORY_USAGE_CPU_TO_GPU);

		m_DescriptorInfo.buffer = m_Buffer.Buffer;
		m_DescriptorInfo.offset = 0;
		m_DescriptorInfo.range = m_Size;
	}

	VulkanBufferUniform::~VulkanBufferUniform()
	{
		VulkanAllocator::DestroyBuffer(m_Buffer);
	}

	void VulkanBufferUniform::WriteData(const void* data, const uint32_t size) const
	{
		memcpy(m_Buffer.AllocationInfo.pMappedData, data, size);
	}
}
