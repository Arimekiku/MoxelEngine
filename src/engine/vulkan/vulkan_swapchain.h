#pragma once

#include "vulkan_command_queue.h"

#include <deque>
#include <functional>

namespace SDLarria 
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

		void QueueResize(std::function<void()>&& function) { m_ResizeQueue.push_back(function); }

		void Initialize(const VkExtent2D& windowSize);
		void Resize();
		void Destroy();

		VkExtent2D& GetSwapchainSize() { return m_SwapchainExtent; }
		VkFormat& GetImageFormat() { return m_SwapchainImageFormat; }

		void UpdateFrame(const CommandBufferData& reservedBuffer);
		void ShowSwapchain(const CommandBufferData& reservedBuffer);
		FrameData& GetCurrentFrame() { return m_CurrentFrame; }

	private:
		std::vector<VkImage> m_Images;
		std::vector<VkImageView> m_ImageViews;

		uint32_t m_CurrentFrameIndex = 0;
		FrameData m_CurrentFrame;

		VkDevice m_DeviceInstance = nullptr;
		VkSwapchainKHR m_SwapchainInstance = nullptr;
		VkFormat m_SwapchainImageFormat = VK_FORMAT_UNDEFINED;

		VkExtent2D m_SwapchainExtent = VkExtent2D(0, 0);

		std::deque<std::function<void()>> m_ResizeQueue;

		friend class VulkanRenderer;
	};
}