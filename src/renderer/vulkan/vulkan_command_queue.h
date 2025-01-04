#pragma once

#include <vulkan/vulkan_core.h>
#include <vector>

namespace Moxel
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

		void BeginImmediateQueue() const;
		void EndImmediateQueue() const;

		void BeginCommandQueue() const;
		void EndCommandQueue() const;

		VkCommandBuffer GetImmediateBuffer() const { return  m_ImmediateBuffer;}
		VkCommandBuffer GetOperatingBuffer() const { return m_CurrentBuffer.CommandBuffer; }
		CommandBufferData& GetNextFrame();

	private:
		std::vector<CommandBufferData> m_Buffers = std::vector<CommandBufferData>();
		CommandBufferData m_CurrentBuffer;

		VkFence m_ImmediateFence = nullptr;
		VkCommandPool m_ImmediatePool = nullptr;
		VkCommandBuffer m_ImmediateBuffer = nullptr;

		VkDevice m_DeviceInstance = nullptr;
		VkQueue m_GraphicsQueue = nullptr;
		uint32_t m_QueueFamily = 0;

		friend class VulkanRenderer;
	};
}