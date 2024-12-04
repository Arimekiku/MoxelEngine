#pragma once

#include <vulkan/vulkan_core.h>

namespace SDLarria {
	class VulkanImage {
	public:
		static void WriteImage(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout);
	};
}