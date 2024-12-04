#pragma once

#include <VkBootstrap.h>
#include <vector>

namespace SDLarria {
	class VulkanSwapchain {
	public:
		VulkanSwapchain() = default;

		void Initialize(vkb::Device& device, VkSurfaceKHR surface, VkExtent2D& surfaceExtent);

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