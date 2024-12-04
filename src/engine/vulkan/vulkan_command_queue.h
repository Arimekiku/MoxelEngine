#pragma once

#include "vulkan_instance.h"

#include <vulkan/vulkan_core.h>

namespace SDLarria 
{
	struct FrameData 
	{
		VkCommandPool CommandPool = nullptr;
		VkCommandBuffer CommandBuffer = nullptr;

		VkSemaphore SwapchainSemaphore = nullptr; 
		VkSemaphore RenderSemaphore = nullptr;
		VkFence RenderFence = nullptr;

		FrameData() = default;
	};

	constexpr unsigned int FRAME_OVERLAP = 2;

	class VulkanCommandPool 
	{
	public:
		VulkanCommandPool() = default;
		
		void Initialize(const VulkanInstance& toolset);
		void Destroy() const;

		FrameData& GetLastFrame() { return m_Frames[m_CurrentFrame % FRAME_OVERLAP]; };

	private:
		FrameData m_Frames[FRAME_OVERLAP];
		int m_CurrentFrame = 0;

		VkDevice m_DeviceInstance = nullptr;
		VkQueue m_GraphicsQueue = nullptr;
		uint32_t m_QueueFamily = 0;

		friend class VulkanEngine;
	};
}