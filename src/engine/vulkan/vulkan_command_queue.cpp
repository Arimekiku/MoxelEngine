#include "vulkan_command_queue.h"
#include "vulkan.h"

namespace SDLarria 
{
	void VulkanCommandPool::Initialize(const VulkanInstance& toolset)
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
		for (auto& m_Frame : m_Frames)
		{
			m_Frame = FrameData();

			auto result = vkCreateCommandPool(m_DeviceInstance, &commandPoolInfo, nullptr, &m_Frame.CommandPool);
			VULKAN_CHECK(result);

			auto cmdAllocInfo = VkCommandBufferAllocateInfo();
			cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			cmdAllocInfo.pNext = nullptr;
			cmdAllocInfo.commandPool = m_Frame.CommandPool;
			cmdAllocInfo.commandBufferCount = 1;
			cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

			result = vkAllocateCommandBuffers(m_DeviceInstance, &cmdAllocInfo, &m_Frame.CommandBuffer);
			VULKAN_CHECK(result);
		}

		auto fenceCreateInfo = VkFenceCreateInfo();
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCreateInfo.pNext = nullptr;
		fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		auto semaphoreCreateInfo = VkSemaphoreCreateInfo();
		semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphoreCreateInfo.pNext = nullptr;

		// Allocate fences and semaphores
		for (auto& m_Frame : m_Frames)
		{
			auto result = vkCreateFence(m_DeviceInstance, &fenceCreateInfo, nullptr, &m_Frame.RenderFence);
			VULKAN_CHECK(result);

			result = vkCreateSemaphore(m_DeviceInstance, &semaphoreCreateInfo, nullptr, &m_Frame.SwapchainSemaphore);
			VULKAN_CHECK(result);

			result = vkCreateSemaphore(m_DeviceInstance, &semaphoreCreateInfo, nullptr, &m_Frame.RenderSemaphore);
			VULKAN_CHECK(result);
		}
	}

	void VulkanCommandPool::Destroy() const 
	{
		vkDeviceWaitIdle(m_DeviceInstance);

		for (const auto& m_Frame : m_Frames)
		{
			vkDestroyCommandPool(m_DeviceInstance, m_Frame.CommandPool, nullptr);
			vkDestroyFence(m_DeviceInstance, m_Frame.RenderFence, nullptr);
			vkDestroySemaphore(m_DeviceInstance, m_Frame.RenderSemaphore, nullptr);
			vkDestroySemaphore(m_DeviceInstance, m_Frame.SwapchainSemaphore, nullptr);
		}
	}
}