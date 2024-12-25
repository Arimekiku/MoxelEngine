#pragma once

#include "vulkan_instance.h"
#include "vulkan_command_queue.h"

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

		void Initialize(const VulkanInstance& toolset, const VkExtent2D& windowSize);
		void Destroy();

		VkExtent2D& GetSwapchainSize() { return m_SwapchainExtent; }
		VkFormat& GetImageFormat() { return m_SwapchainImageFormat; }

		FrameData& GetCurrentFrame(const CommandBufferData& reservedBuffer);
		void ShowSwapchain(const CommandBufferData& reservedBuffer) const;

	private:
		std::vector<FrameData> m_Frames = std::vector<FrameData>();

		uint32_t m_CurrentFrameIndex = 0;
		FrameData m_CurrentFrame;

		VkDevice m_DeviceInstance = nullptr;
		VkSwapchainKHR m_SwapchainInstance = nullptr;
		VkFormat m_SwapchainImageFormat = VK_FORMAT_UNDEFINED;

		VkExtent2D m_SwapchainExtent = VkExtent2D(0, 0);

		friend class VulkanRenderer;
	};
}