#pragma once

#include "renderer/core/asset.h"

#include <vk_mem_alloc.h>

namespace Moxel
{
	struct VulkanImageSpecs
	{
		VkFormat Format = VK_FORMAT_UNDEFINED;
		VkExtent2D InitialSize = { 600, 800 };
		VkImageUsageFlags ImageUsages = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		VkImageAspectFlags ImageAspects = VK_IMAGE_ASPECT_COLOR_BIT;
	};

	struct VulkanImage final : Asset
	{
		VkImageLayout Layout = VK_IMAGE_LAYOUT_UNDEFINED;
		VkImage Image = nullptr;
		VkImageView ImageView = nullptr;
		VkFormat ImageFormat = VK_FORMAT_UNDEFINED;
		VkExtent3D ImageExtent = { 0, 0, 0 };

		void copy_into(const VulkanImage& target) const;
		void copy_raw(VkImage target, const VkExtent2D& imageSize) const;
		static void transit(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);
	};
}