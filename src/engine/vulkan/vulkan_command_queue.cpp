#include "vulkan_command_queue.h"
#include "vulkan.h"

namespace SDLarria 
{
	void VulkanCommandBuffer::Initialize(const VulkanInstance& toolset, const int frameCount)
	{
		m_DeviceInstance = toolset.GetLogicalDevice();
		m_GraphicsQueue = toolset.GetRenderQueue();
		m_QueueFamily = toolset.GetQueueFamilyIndex();

		auto commandPoolInfo = VkCommandPoolCreateInfo();
		commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandPoolInfo.pNext = nullptr;
		commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		commandPoolInfo.queueFamilyIndex = m_QueueFamily;

		// Allocate buffers
		m_Buffers.resize(frameCount);
		for (auto& frame : m_Buffers)
		{
			frame = CommandBufferData();

			auto result = vkCreateCommandPool(m_DeviceInstance, &commandPoolInfo, nullptr, &frame.CommandPool);
			VULKAN_CHECK(result);

			auto cmdAllocInfo = VkCommandBufferAllocateInfo();
			cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			cmdAllocInfo.pNext = nullptr;
			cmdAllocInfo.commandPool = frame.CommandPool;
			cmdAllocInfo.commandBufferCount = 1;
			cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

			result = vkAllocateCommandBuffers(m_DeviceInstance, &cmdAllocInfo, &frame.CommandBuffer);
			VULKAN_CHECK(result);
		}

		// Allocate fences and semaphores
		auto fenceCreateInfo = VkFenceCreateInfo();
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCreateInfo.pNext = nullptr;
		fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		auto semaphoreCreateInfo = VkSemaphoreCreateInfo();
		semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphoreCreateInfo.pNext = nullptr;

		for (auto& frame : m_Buffers)
		{
			auto result = vkCreateFence(m_DeviceInstance, &fenceCreateInfo, nullptr, &frame.RenderFence);
			VULKAN_CHECK(result);

			result = vkCreateSemaphore(m_DeviceInstance, &semaphoreCreateInfo, nullptr, &frame.SwapchainSemaphore);
			VULKAN_CHECK(result);

			result = vkCreateSemaphore(m_DeviceInstance, &semaphoreCreateInfo, nullptr, &frame.RenderSemaphore);
			VULKAN_CHECK(result);
		}
	}

	void VulkanCommandBuffer::Destroy() const 
	{
		for (const auto& frame : m_Buffers)
		{
			vkDestroyCommandPool(m_DeviceInstance, frame.CommandPool, nullptr);
			vkDestroyFence(m_DeviceInstance, frame.RenderFence, nullptr);
			vkDestroySemaphore(m_DeviceInstance, frame.RenderSemaphore, nullptr);
			vkDestroySemaphore(m_DeviceInstance, frame.SwapchainSemaphore, nullptr);
		}
	}
}