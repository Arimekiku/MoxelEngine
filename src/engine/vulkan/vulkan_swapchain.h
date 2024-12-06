#pragma once

#include "vulkan_instance.h"

#include <vector>

namespace SDLarria 
{
	class VulkanSwapchain 
	{
	public:
		VulkanSwapchain() = default;

		VkExtent2D GetSwapchainSize() { return m_SwapchainExtent; }

		void Initialize(const VulkanInstance& toolset, const VkExtent2D& windowSize);
		void Destroy();

	private:
		VkDevice m_DeviceInstance = nullptr;
		VkSwapchainKHR m_SwapchainInstance = nullptr;
		VkFormat m_SwapchainImageFormat = VK_FORMAT_UNDEFINED;

		std::vector<VkImage> m_Images;
		std::vector<VkImageView> m_ImageViews;
		VkExtent2D m_SwapchainExtent = VkExtent2D(0, 0);

		friend class VulkanEngine;
	};
}