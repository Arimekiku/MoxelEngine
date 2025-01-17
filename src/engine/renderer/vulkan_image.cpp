#include "vulkan_image.h"
#include "vulkan.h"
#include "vulkan_allocator.h"
#include "vulkan_renderer.h"
#include "engine/application.h"

#include <backends/imgui_impl_vulkan.h>
#include <stb_image.h>

namespace Moxel
{
	VulkanImage::VulkanImage(VulkanImageSpecs specs, bool storeTexture)
	{
		const auto device = Application::get().get_context().get_logical_device();
		auto& allocator = Application::get().get_allocator();

		m_asset.ImageFormat = specs.Format;
		m_asset.ImageExtent = VkExtent3D(specs.InitialSize.width, specs.InitialSize.height, 1);
		m_aspect = specs.ImageAspects;

		auto imageInfo = VkImageCreateInfo();
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.pNext = nullptr;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.format = m_asset.ImageFormat;
		imageInfo.extent = m_asset.ImageExtent;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.usage = specs.ImageUsages;

		m_asset = allocator.allocate_image(imageInfo, VMA_MEMORY_USAGE_GPU_ONLY);

		auto imageViewInfo = VkImageViewCreateInfo();
		imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewInfo.pNext = nullptr;
		imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewInfo.image = m_asset.Image;
		imageViewInfo.format = m_asset.ImageFormat;
		imageViewInfo.subresourceRange.baseMipLevel = 0;
		imageViewInfo.subresourceRange.levelCount = 1;
		imageViewInfo.subresourceRange.baseArrayLayer = 0;
		imageViewInfo.subresourceRange.layerCount = 1;
		imageViewInfo.subresourceRange.aspectMask = m_aspect;

		auto result = vkCreateImageView(device, &imageViewInfo, nullptr, &m_asset.ImageView);
		VULKAN_CHECK(result);

		// create descriptor set for imgui
		if (storeTexture == false)
		{
			return;
		}

		auto samplerInfo = VkSamplerCreateInfo();
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.minLod = -1000;
		samplerInfo.maxLod = 1000;
		samplerInfo.maxAnisotropy = 1.0f;

		result = vkCreateSampler(device, &samplerInfo, nullptr, &m_sampler);
		VULKAN_CHECK(result);

		m_imageId = ImGui_ImplVulkan_AddTexture(m_sampler, m_asset.ImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

	VulkanImage::VulkanImage(const char* path)
	{
		const auto device = Application::get().get_context().get_logical_device();
		auto& allocator = Application::get().get_allocator();

		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load(path, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

		if (!pixels)
		{
			LOG_ASSERT(false, "Couldn't load image from path {}", path);
		}

		const void* pPixels = pixels;
		const VkDeviceSize imageSize = texWidth * texHeight * 4;

		// allocate temporary buffer for holding texture data to upload
		auto stagingBufferInfo = VkBufferCreateInfo();
		stagingBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		stagingBufferInfo.pNext = nullptr;
		stagingBufferInfo.size = imageSize;
		stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

		constexpr auto stagingMemoryUsage = VMA_MEMORY_USAGE_CPU_ONLY;

		const auto stagingBuffer = allocator.allocate_buffer(stagingBufferInfo, stagingMemoryUsage);

		// copy data to buffer
		void* data = stagingBuffer.AllocationInfo.pMappedData;
		memcpy(data, pPixels, imageSize);

		stbi_image_free(pixels);

		auto drawImageUsages = VkImageUsageFlags();
		drawImageUsages |= VK_IMAGE_USAGE_SAMPLED_BIT;
		drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

		const auto [width, height] = VkExtent2D(static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

		m_asset.ImageFormat = VK_FORMAT_R8G8B8A8_SRGB;
		m_asset.ImageExtent = VkExtent3D(width, height, 1);
		m_aspect = VK_IMAGE_ASPECT_COLOR_BIT;

		auto imageInfo = VkImageCreateInfo();
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.pNext = nullptr;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.format = m_asset.ImageFormat;
		imageInfo.extent = m_asset.ImageExtent;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.usage = drawImageUsages;

		m_asset = allocator.allocate_image(imageInfo, VMA_MEMORY_USAGE_GPU_ONLY);

		auto imageViewInfo = VkImageViewCreateInfo();
		imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewInfo.pNext = nullptr;
		imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewInfo.image = m_asset.Image;
		imageViewInfo.format = m_asset.ImageFormat;
		imageViewInfo.subresourceRange.baseMipLevel = 0;
		imageViewInfo.subresourceRange.levelCount = 1;
		imageViewInfo.subresourceRange.baseArrayLayer = 0;
		imageViewInfo.subresourceRange.layerCount = 1;
		imageViewInfo.subresourceRange.aspectMask = m_aspect;

		auto result = vkCreateImageView(device, &imageViewInfo, nullptr, &m_asset.ImageView);
		VULKAN_CHECK(result);

		VulkanRenderer::immediate_submit(
		[&](const VkCommandBuffer cmd)
		{
			VkImageSubresourceRange range;
			range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			range.baseMipLevel = 0;
			range.levelCount = 1;
			range.baseArrayLayer = 0;
			range.layerCount = 1;

			auto transferBarrier = VkImageMemoryBarrier();
			transferBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			transferBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			transferBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			transferBarrier.image = m_asset.Image;
			transferBarrier.subresourceRange = range;
			transferBarrier.srcAccessMask = 0;
			transferBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			// barrier the image into the transfer-receive layout
			vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0,
								 nullptr, 0, nullptr, 1, &transferBarrier);

			VkBufferImageCopy copyRegion = {};
			copyRegion.bufferOffset = 0;
			copyRegion.bufferRowLength = 0;
			copyRegion.bufferImageHeight = 0;

			copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			copyRegion.imageSubresource.mipLevel = 0;
			copyRegion.imageSubresource.baseArrayLayer = 0;
			copyRegion.imageSubresource.layerCount = 1;
			copyRegion.imageExtent = m_asset.ImageExtent;

			// copy the buffer into the image
			vkCmdCopyBufferToImage(cmd, stagingBuffer.Buffer, m_asset.Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

			auto readableBarrier = transferBarrier;
			readableBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			readableBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			readableBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			readableBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			// barrier the image into the shader readable layout
			vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
								 0, nullptr, 0, nullptr, 1, &readableBarrier);
		});

		allocator.destroy_buffer(stagingBuffer);
		 
		// create descriptor set for imgui
		auto samplerInfo = VkSamplerCreateInfo();
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.minLod = -1000;
		samplerInfo.maxLod = 1000;
		samplerInfo.maxAnisotropy = 1.0f;

		result = vkCreateSampler(device, &samplerInfo, nullptr, &m_sampler);
		VULKAN_CHECK(result);

		m_imageId = ImGui_ImplVulkan_AddTexture(m_sampler, m_asset.ImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

	VulkanImage::~VulkanImage()
	{
		const auto device = Application::get().get_context().get_logical_device();

		vkDestroySampler(device, m_sampler, nullptr);

		if (m_imageId != nullptr)
		{
			ImGui_ImplVulkan_RemoveTexture(m_imageId);
		}

		Application::get().get_allocator().destroy_image(m_asset);
	}

	void VulkanImage::copy_into(const ImageAsset& target) const
	{
		auto blitRegion = VkImageBlit2();
		blitRegion.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2;
		blitRegion.pNext = nullptr;

		blitRegion.srcOffsets[1].x = m_asset.ImageExtent.width;
		blitRegion.srcOffsets[1].y = m_asset.ImageExtent.height;
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
		blitInfo.srcImage = m_asset.Image;
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

		blitRegion.srcOffsets[1].x = m_asset.ImageExtent.width;
		blitRegion.srcOffsets[1].y = m_asset.ImageExtent.height;
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
		blitInfo.srcImage = m_asset.Image;
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

	void VulkanImage::clear(const glm::vec3 color) const
	{
		transit(m_asset.Image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

		auto vulkanSubresourceRange = VkImageSubresourceRange();
		vulkanSubresourceRange.aspectMask = m_aspect;
		vulkanSubresourceRange.levelCount = 1;
		vulkanSubresourceRange.layerCount = 1;

		const auto clearColor = VkClearColorValue
		{
			color.r, color.g, color.b, 0.0f
		};

		const auto buffer = VulkanRenderer::get_command_pool().get_operating_buffer();
		vkCmdClearColorImage(buffer, m_asset.Image, VK_IMAGE_LAYOUT_GENERAL, &clearColor, 1, &vulkanSubresourceRange);
	}
}