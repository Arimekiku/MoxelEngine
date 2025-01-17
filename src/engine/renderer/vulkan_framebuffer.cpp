#include "vulkan_framebuffer.h"
#include "engine/application.h"

namespace Moxel
{
	VulkanFramebuffer::VulkanFramebuffer()
	{
		const auto& windowSize = Application::get().get_window().get_window_size();

		// setup render infos
		auto drawImageUsages = VkImageUsageFlags();
		drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		drawImageUsages |= VK_IMAGE_USAGE_STORAGE_BIT;
		drawImageUsages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		const auto colorSpecs = VulkanImageSpecs
		{
			.Format = VK_FORMAT_R16G16B16A16_SFLOAT,
			.InitialSize = windowSize,
			.ImageUsages = drawImageUsages,
			.ImageAspects = VK_IMAGE_ASPECT_COLOR_BIT
		};

		m_colorImage = std::make_shared<VulkanImage>(colorSpecs, false);
		m_colorAttachment = VkRenderingAttachmentInfo();
		m_colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		m_colorAttachment.imageView = m_colorImage->get_image_asset().ImageView;
		m_colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		m_colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		m_colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

		const auto depthSpecs = VulkanImageSpecs
		{
			.Format = VK_FORMAT_D32_SFLOAT_S8_UINT,
			.InitialSize = windowSize, 
			.ImageUsages = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			.ImageAspects = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT
		};

		m_depthImage = std::make_shared<VulkanImage>(depthSpecs, false);
		m_depthAttachment = VkRenderingAttachmentInfo();
		m_depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		m_depthAttachment.imageView = m_depthImage->get_image_asset().ImageView;
		m_depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
		m_depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		m_depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		m_depthAttachment.clearValue.depthStencil.depth = 1.0f;
	}

	void VulkanFramebuffer::bind() const
	{
		VulkanImage::transit(m_colorImage->get_image_asset().Image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		VulkanImage::transit(m_depthImage->get_image_asset().Image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
	}
}