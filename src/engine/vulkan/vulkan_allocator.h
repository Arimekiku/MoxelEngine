#pragma once

#include <memory>

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
		static void DestroyVulkanImage(const std::shared_ptr<VulkanImage>& image);

		static void* MapData(VmaAllocation allocation)
		{
			void* mappedMemory = nullptr;
			vmaMapMemory(m_Allocator, allocation, &mappedMemory);
			return mappedMemory;
		}
		static void UnmapData(VmaAllocation allocation);
	private:
		static VmaAllocator m_Allocator;
	};
}
