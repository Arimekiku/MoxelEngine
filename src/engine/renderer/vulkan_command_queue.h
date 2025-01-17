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
		
		void initialize(int bufferCount);
		void destroy() const;

		void begin_immediate_queue() const;
		void end_immediate_queue() const;

		void begin_command_queue() const;
		void end_command_queue() const;

		VkCommandBuffer get_immediate_buffer() const { return m_immediateBuffer;}
		VkCommandBuffer get_operating_buffer() const { return m_currentBuffer.CommandBuffer; }
		CommandBufferData& get_next_frame();

	private:
		std::vector<CommandBufferData> m_buffers = std::vector<CommandBufferData>();
		CommandBufferData m_currentBuffer;

		VkFence m_immediateFence = nullptr;
		VkCommandPool m_immediatePool = nullptr;
		VkCommandBuffer m_immediateBuffer = nullptr;

		VkDevice m_deviceInstance = nullptr;
		VkQueue m_graphicsQueue = nullptr;
		uint32_t m_queueFamily = 0;
	};
}