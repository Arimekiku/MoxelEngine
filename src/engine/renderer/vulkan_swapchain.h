#pragma once

#include "vulkan_command_queue.h"
#include "vulkan_framebuffer.h"

namespace Moxel
{
	struct FrameData
	{
		VkImage ImageData = nullptr;
		VkImageView ImageViewData = nullptr;
	};

	class VulkanSwapchain 
	{
	public:
		VulkanSwapchain() = default;

		void initialize(const VkExtent2D& windowSize);
		void resize();
		void destroy();

		VkExtent2D& get_swapchain_size() { return m_swapchainExtent; }
		VkFormat& get_image_format() { return m_swapchainImageFormat; }

		void update_frame(const CommandBufferData& reservedBuffer);
		void show_swapchain(const CommandBufferData& reservedBuffer);
		FrameData& get_current_frame() { return m_currentFrame; }
		const std::shared_ptr<VulkanFramebuffer>& get_framebuffer() const { return m_framebuffer; }

	private:
		std::vector<VkImage> m_images;
		std::vector<VkImageView> m_imageViews;

		uint32_t m_currentFrameIndex = 0;
		FrameData m_currentFrame;
		std::shared_ptr<VulkanFramebuffer> m_framebuffer;

		VkDevice m_deviceInstance = nullptr;
		VkSwapchainKHR m_swapchainInstance = nullptr;
		VkFormat m_swapchainImageFormat = VK_FORMAT_UNDEFINED;

		VkExtent2D m_swapchainExtent = { 0, 0 };
	};
}
