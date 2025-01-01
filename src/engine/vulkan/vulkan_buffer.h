#pragma once

#include <vulkan/vulkan_core.h>
#include <vk_mem_alloc.h>

namespace SDLarria
{
	struct VulkanBuffer
	{
		VkBuffer Buffer = nullptr;
		VmaAllocation Allocation = nullptr;
		VmaAllocationInfo AllocationInfo = VmaAllocationInfo();
	};
}