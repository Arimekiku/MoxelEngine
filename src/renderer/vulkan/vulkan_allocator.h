#pragma once

#include <unordered_map>

#include "vulkan_buffer.h"
#include "vulkan_image.h"

namespace Moxel
{
	class VulkanAllocator
	{
	public:
		VulkanAllocator() = default;

		void initialize();
		void destroy() const;

		VulkanBuffer allocate_buffer(const VkBufferCreateInfo& bufferCreateInfo, VmaMemoryUsage usage);
		VulkanImage allocate_image(const VulkanImageSpecs& specs, VmaMemoryUsage usage);

		void destroy_buffer(const VulkanBuffer& buffer);
		void destroy_vulkan_image(const VulkanImage& image);
	private:
		VmaAllocator m_allocator;

		std::unordered_map<UUID, VmaAllocation> m_allocatorAssets;
	};
}
