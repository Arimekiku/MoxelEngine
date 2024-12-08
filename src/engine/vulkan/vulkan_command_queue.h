#pragma once

#include "vulkan_instance.h"

#include <vulkan/vulkan_core.h>
#include <vector>

namespace SDLarria 
{
	struct CommandBufferData 
	{
		VkCommandPool CommandPool = nullptr;
		VkCommandBuffer CommandBuffer = nullptr;

		VkSemaphore SwapchainSemaphore = nullptr;
		VkSemaphore RenderSemaphore = nullptr;
		VkFence RenderFence = nullptr;

		CommandBufferData() = default;
	};

	class VulkanCommandBuffer 
	{
	public:
		VulkanCommandBuffer() = default;
		
		void Initialize(const VulkanInstance& toolset, const int bufferCount);
		void Destroy() const;

		CommandBufferData& GetCurrentFrame() { return m_CurrentBuffer; }
		CommandBufferData& GetFrame(int index) { return m_Buffers[index]; }

	private:
		std::vector<CommandBufferData> m_Buffers = std::vector<CommandBufferData>();
		CommandBufferData m_CurrentBuffer;

		VkDevice m_DeviceInstance = nullptr;
		VkQueue m_GraphicsQueue = nullptr;
		uint32_t m_QueueFamily = 0;

		friend class VulkanEngine;
	};
}