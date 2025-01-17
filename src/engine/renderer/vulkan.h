#pragma once

#include "engine/core/core.h"

#include <vulkan/vk_enum_string_helper.h>

namespace Moxel
{
	class VulkanUtils 
	{
	public: 
		static void vulkan_check(const VkResult result)
		{
			if (result != VK_SUCCESS)
			{
				LOG_CRITICAL("Vulkan error: {0}", string_VkResult(result));
			}

			LOG_ASSERT((result == VK_SUCCESS), "Vulkan");
		}
	};
}

#define VULKAN_CHECK(x)	::Moxel::VulkanUtils::vulkan_check(x);