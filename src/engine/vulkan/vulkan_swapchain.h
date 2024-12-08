#pragma once

#include "vulkan_instance.h"

#include <vector>

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

		VkExtent2D& GetSwapchainSize() { return m_SwapchainExtent; }
		VkFormat& GetImageFormat() { return m_SwapchainImageFormat; }

		FrameData& GetCurrentFrame() { return m_CurrentFrame; }
		FrameData& GetFrame(int index) { return m_Frames[index]; }

		void Initialize(const VulkanInstance& toolset, const VkExtent2D& windowSize);
		void Destroy();

	private:
		std::vector<FrameData> m_Frames = std::vector<FrameData>();
		FrameData m_CurrentFrame;

		VkDevice m_DeviceInstance = nullptr;
		VkSwapchainKHR m_SwapchainInstance = nullptr;
		VkFormat m_SwapchainImageFormat = VK_FORMAT_UNDEFINED;

		VkExtent2D m_SwapchainExtent = VkExtent2D(0, 0);

		friend class VulkanEngine;
	};
}