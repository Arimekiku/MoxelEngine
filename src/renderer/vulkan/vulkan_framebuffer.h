#pragma once

#include "vulkan_image.h"
#include "engine/render_mesh.h"

namespace Moxel
{
	class VulkanFramebuffer
	{
	public:
		VulkanFramebuffer();
		~VulkanFramebuffer();

		void bind() const;

		const std::shared_ptr<VulkanImage>& get_render_image() const { return m_colorImage; }
		const std::shared_ptr<VulkanImage>& get_depth_image() const { return m_depthImage; }

		const VkRenderingAttachmentInfo& get_color_attachment() const { return m_colorAttachment; }
		const VkRenderingAttachmentInfo& get_depth_attachment() const { return m_depthAttachment; }

	private:
		std::shared_ptr<VulkanImage> m_colorImage;
		VkRenderingAttachmentInfo m_colorAttachment;

		std::shared_ptr<VulkanImage> m_depthImage;
		VkRenderingAttachmentInfo m_depthAttachment;
	};
}
