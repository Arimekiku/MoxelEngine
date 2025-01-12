#pragma once

#include <unordered_map>

#include "vulkan_buffer.h"
#include "vulkan_image.h"

namespace Moxel
{
	class VulkanAllocator
	{
	public:
		VulkanAllocator() = delete;

		static void initialize();
		static void destroy();

		static VulkanBuffer allocate_buffer(const VkBufferCreateInfo& bufferCreateInfo, VmaMemoryUsage usage);
		static VulkanImage allocate_image(const VulkanImageSpecs& specs, VmaMemoryUsage usage);

		static void destroy_buffer(const VulkanBuffer& buffer);
		static void destroy_vulkan_image(const VulkanImage& image);
	private:
		static VmaAllocator m_allocator;

		static std::unordered_map<UUID, VmaAllocation> m_allocatorAssets;
	};
}
