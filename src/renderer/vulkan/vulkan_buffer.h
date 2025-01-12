#pragma once

#include "renderer/core/asset.h"

#include <vk_mem_alloc.h>

namespace Moxel
{
	struct VulkanBuffer final : Asset
	{
		VkBuffer Buffer = nullptr;
		VmaAllocation Allocation = nullptr;
		VmaAllocationInfo AllocationInfo = VmaAllocationInfo();
	};
}
