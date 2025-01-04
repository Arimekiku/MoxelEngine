#pragma once

#include "vulkan_command_queue.h"
#include "vulkan_image.h"
#include "vulkan_framebuffer.h"

#include <memory>

namespace Moxel
{
	struct FrameData
	{
		VkImage ImageData = nullptr;
		VkImageView ImageViewData = nullptr;

		FrameData() = default;
	};

	class VulkanSwapchain 
	{
	public:
		VulkanSwapchain() = default;

		void Initialize(const VkExtent2D& windowSize);
		void Resize();
		void Destroy();

		VkExtent2D& GetSwapchainSize() { return m_SwapchainExtent; }
		VkFormat& GetImageFormat() { return m_SwapchainImageFormat; }

		void UpdateFrame(const CommandBufferData& reservedBuffer);
		void ShowSwapchain(const CommandBufferData& reservedBuffer);
		FrameData& GetCurrentFrame() { return m_CurrentFrame; }
		const std::shared_ptr<VulkanFramebuffer>& GetFramebuffer() const { return m_Framebuffer; }

	private:
		std::vector<VkImage> m_Images;
		std::vector<VkImageView> m_ImageViews;

		uint32_t m_CurrentFrameIndex = 0;
		FrameData m_CurrentFrame;
		std::shared_ptr<VulkanFramebuffer> m_Framebuffer;

		VkDevice m_DeviceInstance = nullptr;
		VkSwapchainKHR m_SwapchainInstance = nullptr;
		VkFormat m_SwapchainImageFormat = VK_FORMAT_UNDEFINED;

		VkExtent2D m_SwapchainExtent = VkExtent2D(0, 0);

		friend class VulkanRenderer;
	};
}
