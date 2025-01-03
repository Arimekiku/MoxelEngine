#pragma once

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