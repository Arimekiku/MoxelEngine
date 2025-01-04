#include "vulkan_framebuffer.h"
#include "renderer/application.h"

namespace Moxel
{
	VulkanFramebuffer::VulkanFramebuffer()
	{
		const auto& windowSize = Application::Get().GetWindow().GetWindowSize();

		// setup render infos
		auto drawImageUsages = VkImageUsageFlags();
		drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		drawImageUsages |= VK_IMAGE_USAGE_STORAGE_BIT;
		drawImageUsages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		const auto colorSpecs = VulkanImageSpecs
		{
			VK_FORMAT_R16G16B16A16_SFLOAT,
			windowSize,
			drawImageUsages,
			VK_IMAGE_ASPECT_COLOR_BIT
		};

		m_ColorImage = std::make_shared<VulkanImage>(colorSpecs);
		m_ColorAttachment = VkRenderingAttachmentInfo();
		m_ColorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		m_ColorAttachment.imageView = m_ColorImage->GetImageView();
		m_ColorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		m_ColorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		m_ColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

		const auto depthSpecs = VulkanImageSpecs
		{
			VK_FORMAT_D32_SFLOAT,
			windowSize, 
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			VK_IMAGE_ASPECT_DEPTH_BIT
		};

		m_DepthImage = std::make_shared<VulkanImage>(depthSpecs);
		m_DepthAttachment = VkRenderingAttachmentInfo();
		m_DepthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		m_DepthAttachment.imageView = m_DepthImage->GetImageView();
		m_DepthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
		m_DepthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		m_DepthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		m_DepthAttachment.clearValue.depthStencil.depth = 1.f;
	}

	VulkanFramebuffer::~VulkanFramebuffer() 
	{ 
		m_ColorImage = nullptr;
		m_DepthImage = nullptr;
	}

	void VulkanFramebuffer::Bind() const
	{
		VulkanImage::Transit(m_ColorImage->GetRawImage(), VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

		VulkanImage::Transit(m_DepthImage->GetRawImage(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
	}

}
