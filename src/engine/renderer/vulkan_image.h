#pragma once

#include "engine/core/asset.h"

#include <vk_mem_alloc.h>
#include <glm/glm.hpp>

namespace Moxel
{
	struct ImageAsset final : Asset
	{
		VkImageLayout Layout = VK_IMAGE_LAYOUT_UNDEFINED;
		VkImage Image = nullptr;
		VkImageView ImageView = nullptr;
		VkExtent3D ImageExtent = {0, 0, 0};
		VkFormat ImageFormat = VK_FORMAT_UNDEFINED;
	};

	struct VulkanImageSpecs
	{
		VkFormat Format = VK_FORMAT_UNDEFINED;
		VkExtent2D InitialSize = { 600, 800 };
		VkImageUsageFlags ImageUsages = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		VkImageAspectFlags ImageAspects = VK_IMAGE_ASPECT_COLOR_BIT;
	};

	class VulkanImage
	{
	public:
		VulkanImage(VulkanImageSpecs specs, bool storeTexture);
		VulkanImage(const char* path);
		~VulkanImage();

		VkDescriptorSet get_image_id() const { return m_imageId; }
		const ImageAsset& get_image_asset() const { return m_asset; }

		void copy_into(const ImageAsset& target) const;
		void copy_raw(VkImage target, const VkExtent2D& imageSize) const;
		void clear(glm::vec3 color) const;

		static void transit(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);
	private:
		ImageAsset m_asset;

		VkImageLayout m_oldLayout;
		VkImageAspectFlags m_aspect;
		VkDescriptorSet m_imageId = nullptr;
		VkSampler m_sampler = nullptr;
	};
}