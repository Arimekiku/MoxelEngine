#pragma once

#include "vulkan_buffer.h"
#include "vulkan_image.h"

namespace SDLarria 
{
	class VulkanAllocator
	{
	public:
		VulkanAllocator() = delete;

		static void Initialize();
		static void Destroy();

		static VulkanBuffer AllocateBuffer(const VkBufferCreateInfo& bufferCreateInfo, VmaMemoryUsage usage);
		static void AllocateImage(const VkImageCreateInfo& imageCreateInfo, VmaMemoryUsage usage, VkImage& outImage, VmaAllocation& outAllocation);

		static void DestroyBuffer(const VulkanBuffer& buffer);
		static void DestroyVulkanImage(const VulkanImage& image);

		static void* MapData(VmaAllocation allocation);
		static void UnmapData(VmaAllocation allocation);

	private:
		static VmaAllocator m_Allocator;
	};
}
