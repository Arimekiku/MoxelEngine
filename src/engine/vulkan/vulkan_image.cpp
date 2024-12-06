#include "vulkan_image.h"
#include "vulkan.h"

namespace SDLarria 
{
    VulkanImage::VulkanImage(VkDevice device, VmaAllocator allocator, VkExtent2D& size)
    {
        VkExtent3D drawImageExtent = {
            size.width,
            size.height,
            1
        };

        m_ImageFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
        m_ImageExtent = drawImageExtent;

        auto drawImageUsages = VkImageUsageFlags();
        drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        drawImageUsages |= VK_IMAGE_USAGE_STORAGE_BIT;
        drawImageUsages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        auto rimg_info = VkImageCreateInfo();
        rimg_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        rimg_info.pNext = nullptr;
        rimg_info.imageType = VK_IMAGE_TYPE_2D;
        rimg_info.format = m_ImageFormat;
        rimg_info.extent = m_ImageExtent;
        rimg_info.mipLevels = 1;
        rimg_info.arrayLayers = 1;
        rimg_info.samples = VK_SAMPLE_COUNT_1_BIT;
        rimg_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        rimg_info.usage = drawImageUsages;

        auto rimg_allocinfo = VmaAllocationCreateInfo();
        rimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        rimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        vmaCreateImage(allocator, &rimg_info, &rimg_allocinfo, &m_Image, &m_Allocation, nullptr);

        VkImageViewCreateInfo rview_info = VkImageViewCreateInfo();
        rview_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        rview_info.pNext = nullptr;
        rview_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        rview_info.image = m_Image;
        rview_info.format = m_ImageFormat;
        rview_info.subresourceRange.baseMipLevel = 0;
        rview_info.subresourceRange.levelCount = 1;
        rview_info.subresourceRange.baseArrayLayer = 0;
        rview_info.subresourceRange.layerCount = 1;
        rview_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

        auto result = vkCreateImageView(device, &rview_info, nullptr, &m_ImageView);
        VULKAN_CHECK(result);
    }

    void VulkanImage::Copy(VkCommandBuffer cmd, VulkanImage& target) const
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

        vkCmdBlitImage2(cmd, &blitInfo);
    }

    void VulkanImage::Copy(VkCommandBuffer cmd, VkImage target, VkExtent2D imageSize)
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

        vkCmdBlitImage2(cmd, &blitInfo);
    }

    void VulkanImage::Transit(VkCommandBuffer cmd, VkImageLayout newLayout)
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

        vkCmdPipelineBarrier2(cmd, &depInfo);

        m_Layout = newLayout;
	}

    void VulkanImage::Transit(VkCommandBuffer cmd, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout)
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

        vkCmdPipelineBarrier2(cmd, &depInfo);
    }
}