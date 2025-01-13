#include "vulkan_buffer_uniform.h"
#include "vulkan_renderer.h"
#include "renderer/application.h"

namespace Moxel
{
	struct GlobalRenderData;

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
		Application::get().get_allocator().destroy_buffer(m_buffer);
	}

	void VulkanBufferUniform::write_data(const void* data, const uint32_t size) const
	{
		memcpy(m_buffer.AllocationInfo.pMappedData, data, size);
	}
}
