#pragma once

#include "engine/core/core.h"

#include <vulkan/vk_enum_string_helper.h>

namespace SDLarria 
{
	class VulkanUtils 
	{
	public: 
		static void VulkanCheck(VkResult x) 
		{
			if (x != VK_SUCCESS) 
			{
				LOG_CRITICAL("Vulkan error: {0}", string_VkResult(x));
			}

			LOG_ASSERT(x == VK_SUCCESS, "Vulkan");
		}
	};
}

#define VULKAN_CHECK(x)	::SDLarria::VulkanUtils::VulkanCheck(x);