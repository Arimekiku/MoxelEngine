#include "vulkan_image.h"
#include "vulkan.h"
#include "vulkan_allocator.h"
#include "vulkan_renderer.h"
#include "renderer/application.h"

namespace Moxel
{
	void VulkanImage::copy_into(const VulkanImage& target) const
	{
		auto blitRegion = VkImageBlit2();
		blitRegion.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2;
		blitRegion.pNext = nullptr;

		blitRegion.srcOffsets[1].x = this->ImageExtent.width;
		blitRegion.srcOffsets[1].y = this->ImageExtent.height;
		blitRegion.srcOffsets[1].z = 1;

		blitRegion.dstOffsets[1].x = target.ImageExtent.width;
		blitRegion.dstOffsets[1].y = target.ImageExtent.height;
		blitRegion.dstOffsets[1].z = 1;

		blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blitRegion.srcSubresource.baseArrayLayer = 0;
		blitRegion.srcSubresource.layerCount = 1;
		blitRegion.srcSubresource.mipLevel = 0;

		blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blitRegion.dstSubresource.baseArrayLayer = 0;
		blitRegion.dstSubresource.layerCount = 1;
		blitRegion.dstSubresource.mipLevel = 0;

		auto blitInfo = VkBlitImageInfo2();
		blitInfo.sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2;
		blitInfo.pNext = nullptr;
		blitInfo.dstImage = target.Image;
		blitInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		blitInfo.srcImage = this->Image;
		blitInfo.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		blitInfo.filter = VK_FILTER_LINEAR;
		blitInfo.regionCount = 1;
		blitInfo.pRegions = &blitRegion;

		const auto currentCommandBuffer = VulkanRenderer::get_command_pool().get_operating_buffer();
		vkCmdBlitImage2(currentCommandBuffer, &blitInfo);
	}

	void VulkanImage::copy_raw(VkImage target, const VkExtent2D& imageSize) const
	{
		auto blitRegion = VkImageBlit2();
		blitRegion.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2;
		blitRegion.pNext = nullptr;

		blitRegion.srcOffsets[1].x = this->ImageExtent.width;
		blitRegion.srcOffsets[1].y = this->ImageExtent.height;
		blitRegion.srcOffsets[1].z = 1;

		blitRegion.dstOffsets[1].x = imageSize.width;
		blitRegion.dstOffsets[1].y = imageSize.height;
		blitRegion.dstOffsets[1].z = 1;

		blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blitRegion.srcSubresource.baseArrayLayer = 0;
		blitRegion.srcSubresource.layerCount = 1;
		blitRegion.srcSubresource.mipLevel = 0;

		blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blitRegion.dstSubresource.baseArrayLayer = 0;
		blitRegion.dstSubresource.layerCount = 1;
		blitRegion.dstSubresource.mipLevel = 0;

		auto blitInfo = VkBlitImageInfo2();
		blitInfo.sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2;
		blitInfo.pNext = nullptr;
		blitInfo.dstImage = target;
		blitInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		blitInfo.srcImage = this->Image;
		blitInfo.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		blitInfo.filter = VK_FILTER_LINEAR;
		blitInfo.regionCount = 1;
		blitInfo.pRegions = &blitRegion;

		const auto currentCommandBuffer = VulkanRenderer::get_command_pool().get_operating_buffer();
		vkCmdBlitImage2(currentCommandBuffer, &blitInfo);
	}

	void VulkanImage::transit(VkImage image, const VkImageLayout oldLayout, const VkImageLayout newLayout)
	{
		auto imageBarrier = VkImageMemoryBarrier2();
		imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
		imageBarrier.pNext = nullptr;
		imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
		imageBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
		imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
		imageBarrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;
		imageBarrier.oldLayout = oldLayout;
		imageBarrier.newLayout = newLayout;

		auto subImage = VkImageSubresourceRange();
		subImage.aspectMask = (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) ? VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		subImage.baseMipLevel = 0;
		subImage.levelCount = VK_REMAINING_MIP_LEVELS;
		subImage.baseArrayLayer = 0;
		subImage.layerCount = VK_REMAINING_ARRAY_LAYERS;

		imageBarrier.subresourceRange = subImage;
		imageBarrier.image = image;

		auto depInfo = VkDependencyInfo();
		depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		depInfo.pNext = nullptr;
		depInfo.imageMemoryBarrierCount = 1;
		depInfo.pImageMemoryBarriers = &imageBarrier;

		const auto currentCommandBuffer = VulkanRenderer::get_command_pool().get_operating_buffer();
		vkCmdPipelineBarrier2(currentCommandBuffer, &depInfo);
	}
}