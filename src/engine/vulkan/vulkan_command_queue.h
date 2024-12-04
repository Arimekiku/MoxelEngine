#pragma once

#include <vulkan/vulkan_core.h>
#include <VkBootstrap.h>

namespace SDLarria {
	struct FrameData {
		VkCommandPool CommandPool;
		VkCommandBuffer CommandBuffer;

		VkSemaphore SwapchainSemaphore, RenderSemaphore;
		VkFence RenderFence;
	};

	constexpr unsigned int FRAME_OVERLAP = 2;

	class VulkanCommandPool {
	public:
		VulkanCommandPool() = default;
		
		void Initialize(vkb::Device& device);
		void Destroy();

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