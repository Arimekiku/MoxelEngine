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

		void Bind() const;

		const std::shared_ptr<VulkanImage>& GetRenderImage() const { return m_ColorImage; }
		const std::shared_ptr<VulkanImage>& GetDepthImage() const { return m_DepthImage; }

		const VkRenderingAttachmentInfo& GetColorAttachment() const { return m_ColorAttachment; }
		const VkRenderingAttachmentInfo& GetDepthAttachment() const { return m_DepthAttachment; }

	private:
		std::shared_ptr<VulkanImage> m_ColorImage;
		VkRenderingAttachmentInfo m_ColorAttachment;

		std::shared_ptr<VulkanImage> m_DepthImage;
		VkRenderingAttachmentInfo m_DepthAttachment;
	};
}
