#pragma once

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
		
		void Initialize(int bufferCount);
		void Destroy() const;

		void BeginCommandQueue() const;
		void EndCommandQueue() const;

		VkCommandBuffer GetOperatingBuffer() const { return m_CurrentBuffer.CommandBuffer; }
		CommandBufferData& GetNextFrame();

	private:
		std::vector<CommandBufferData> m_Buffers = std::vector<CommandBufferData>();
		CommandBufferData m_CurrentBuffer;

		VkDevice m_DeviceInstance = nullptr;
		VkQueue m_GraphicsQueue = nullptr;
		uint32_t m_QueueFamily = 0;

		friend class VulkanRenderer;
	};
}