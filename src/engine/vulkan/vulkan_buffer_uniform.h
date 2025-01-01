#pragma once

#include "vulkan_buffer.h"

namespace SDLarria
{
	class VulkanBufferUniform
	{
	public:
		VulkanBufferUniform(uint32_t bufferSize);
		~VulkanBufferUniform();

		void WriteData(const void* data, uint32_t size) const;

		const VkDescriptorBufferInfo& GetDescriptorInfo() const { return m_DescriptorInfo; }

	private:
		uint32_t m_Size;

		VulkanBuffer m_Buffer;
		VkDescriptorBufferInfo m_DescriptorInfo;

		void* m_Data;
	};
}