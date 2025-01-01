#include "vulkan_image.h"
#include "vulkan.h"

namespace SDLarria 
{
    VulkanImage::VulkanImage(const VkExtent2D& size)
    {
		const auto device = VulkanRenderer::Get().GetContext().GetLogicalDevice();

        m_ImageFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
        m_ImageExtent =
        {
            size.width,
            size.height,
            1
        };

        auto drawImageUsages = VkImageUsageFlags();
        drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        drawImageUsages |= VK_IMAGE_USAGE_STORAGE_BIT;
        drawImageUsages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        auto imageInfo = VkImageCreateInfo();
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.pNext = nullptr;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.format = m_ImageFormat;
        imageInfo.extent = m_ImageExtent;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.usage = drawImageUsages;

    	VulkanAllocator::AllocateImage(imageInfo, VMA_MEMORY_USAGE_GPU_ONLY, m_Image, m_Allocation);

        auto imageViewInfo = VkImageViewCreateInfo();
        imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewInfo.pNext = nullptr;
        imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewInfo.image = m_Image;
        imageViewInfo.format = m_ImageFormat;
        imageViewInfo.subresourceRange.baseMipLevel = 0;
        imageViewInfo.subresourceRange.levelCount = 1;
        imageViewInfo.subresourceRange.baseArrayLayer = 0;
        imageViewInfo.subresourceRange.layerCount = 1;
        imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

        const auto result = vkCreateImageView(device, &imageViewInfo, nullptr, &m_ImageView);
        VULKAN_CHECK(result);
    }

    void VulkanImage::CopyInto(const VulkanImage& target) const
    {
        auto blitRegion = VkImageBlit2();
        blitRegion.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2;
        blitRegion.pNext = nullptr;

        blitRegion.srcOffsets[1].x = this->m_ImageExtent.width;
        blitRegion.srcOffsets[1].y = this->m_ImageExtent.height;
        blitRegion.srcOffsets[1].z = 1;

        blitRegion.dstOffsets[1].x = target.m_ImageExtent.width;
        blitRegion.dstOffsets[1].y = target.m_ImageExtent.height;
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
        blitInfo.dstImage = target.m_Image;
        blitInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        blitInfo.srcImage = this->m_Image;
        blitInfo.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        blitInfo.filter = VK_FILTER_LINEAR;
        blitInfo.regionCount = 1;
        blitInfo.pRegions = &blitRegion;

        const auto currentCommandBuffer = VulkanRenderer::Get().GetCommandPool().GetOperatingBuffer();
        vkCmdBlitImage2(currentCommandBuffer, &blitInfo);
    }

    void VulkanImage::CopyRaw(VkImage target, const VkExtent2D& imageSize) const
    {
        auto blitRegion = VkImageBlit2();
        blitRegion.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2;
        blitRegion.pNext = nullptr;

        blitRegion.srcOffsets[1].x = this->m_ImageExtent.width;
        blitRegion.srcOffsets[1].y = this->m_ImageExtent.height;
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
        blitInfo.srcImage = this->m_Image;
        blitInfo.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        blitInfo.filter = VK_FILTER_LINEAR;
        blitInfo.regionCount = 1;
        blitInfo.pRegions = &blitRegion;

        const auto currentCommandBuffer = VulkanRenderer::Get().GetCommandPool().GetOperatingBuffer();
        vkCmdBlitImage2(currentCommandBuffer, &blitInfo);
    }

    void VulkanImage::Transit(const VkImageLayout newLayout)
    {
        auto imageBarrier = VkImageMemoryBarrier2();
        imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        imageBarrier.pNext = nullptr;
        imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        imageBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
        imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        imageBarrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;
        imageBarrier.oldLayout = m_Layout;
        imageBarrier.newLayout = newLayout;

        auto subImage = VkImageSubresourceRange();
        subImage.aspectMask = (newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
        subImage.baseMipLevel = 0;
        subImage.levelCount = VK_REMAINING_MIP_LEVELS;
        subImage.baseArrayLayer = 0;
        subImage.layerCount = VK_REMAINING_ARRAY_LAYERS;

        imageBarrier.subresourceRange = subImage;
        imageBarrier.image = m_Image;

        auto depInfo = VkDependencyInfo();
        depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        depInfo.pNext = nullptr;
        depInfo.imageMemoryBarrierCount = 1;
        depInfo.pImageMemoryBarriers = &imageBarrier;

        const auto currentCommandBuffer = VulkanRenderer::Get().GetCommandPool().GetOperatingBuffer();
        vkCmdPipelineBarrier2(currentCommandBuffer, &depInfo);

        m_Layout = newLayout;
	}

    void VulkanImage::Transit(VkImage image, const VkImageLayout oldLayout, const VkImageLayout newLayout)
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
        subImage.aspectMask = (newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
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

        const auto currentCommandBuffer = VulkanRenderer::Get().GetCommandPool().GetOperatingBuffer();
        vkCmdPipelineBarrier2(currentCommandBuffer, &depInfo);
    }
}