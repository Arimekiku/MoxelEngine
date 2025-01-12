#pragma once

#include "vulkan_buffer.h"

namespace Moxel
{
	class VulkanBufferUniform
	{
	public:
		VulkanBufferUniform(uint32_t bufferSize);
		~VulkanBufferUniform();

		void write_data(const void* data, uint32_t size) const;

		const VkDescriptorBufferInfo& get_descriptor_info() const { return m_descriptorInfo; }

	private:
		uint32_t m_size = 0;

		VulkanBuffer m_buffer;
		VkDescriptorBufferInfo m_descriptorInfo;

		void* m_data = nullptr;
	};
}