#include "vulkan_command_queue.h"
#include "engine/core/logger/log.h"

#include <vulkan/vk_enum_string_helper.h>

namespace SDLarria {
	void VulkanCommandPool::Initialize(vkb::Device& device) {
		m_DeviceInstance = device.device;
		m_GraphicsQueue = device.get_queue(vkb::QueueType::graphics).value();
		m_QueueFamily = device.get_queue_index(vkb::QueueType::graphics).value();

		auto commandPoolInfo = VkCommandPoolCreateInfo();
		commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandPoolInfo.pNext = nullptr;
		commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		commandPoolInfo.queueFamilyIndex = m_QueueFamily;

		// Allocate buffers
		for (int i = 0; i < FRAME_OVERLAP; i++) {
			auto result = vkCreateCommandPool(device, &commandPoolInfo, nullptr, &m_Frames[i].CommandPool);
			if (result != VK_SUCCESS) {
				LOG_CRITICAL("Vulkan command pool: {0}", string_VkResult(result));
			}

			auto cmdAllocInfo = VkCommandBufferAllocateInfo();
			cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			cmdAllocInfo.pNext = nullptr;
			cmdAllocInfo.commandPool = m_Frames[i].CommandPool;
			cmdAllocInfo.commandBufferCount = 1;
			cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

			result = vkAllocateCommandBuffers(device, &cmdAllocInfo, &m_Frames[i].CommandBuffer);
			if (result != VK_SUCCESS) {
				LOG_CRITICAL("Vulkan command buffer: {0}", string_VkResult(result));
			}
		}

		auto fenceCreateInfo = VkFenceCreateInfo();
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCreateInfo.pNext = nullptr;
		fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		auto semaphoreCreateInfo = VkSemaphoreCreateInfo();
		semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphoreCreateInfo.pNext = nullptr;

		// Allocate fences and semaphores
		for (int i = 0; i < FRAME_OVERLAP; i++) {
			auto result = vkCreateFence(m_DeviceInstance, &fenceCreateInfo, nullptr, &m_Frames[i].RenderFence);
			if (result != VK_SUCCESS) {
				LOG_CRITICAL("Vulkan fence: {0}", string_VkResult(result));
			}

			result = vkCreateSemaphore(m_DeviceInstance, &semaphoreCreateInfo, nullptr, &m_Frames[i].SwapchainSemaphore);
			if (result != VK_SUCCESS) {
				LOG_CRITICAL("Vulkan semaphore: {0}", string_VkResult(result));
			}

			result = vkCreateSemaphore(m_DeviceInstance, &semaphoreCreateInfo, nullptr, &m_Frames[i].RenderSemaphore);
			if (result != VK_SUCCESS) {
				LOG_CRITICAL("Vulkan semaphore: {0}", string_VkResult(result));
			}
		}
	}

	void VulkanCommandPool::Destroy() {
		vkDeviceWaitIdle(m_DeviceInstance);

		for (int i = 0; i < FRAME_OVERLAP; i++) {
			vkDestroyCommandPool(m_DeviceInstance, m_Frames[i].CommandPool, nullptr);
			vkDestroyFence(m_DeviceInstance, m_Frames[i].RenderFence, nullptr);
			vkDestroySemaphore(m_DeviceInstance, m_Frames[i].RenderSemaphore, nullptr);
			vkDestroySemaphore(m_DeviceInstance, m_Frames[i].SwapchainSemaphore, nullptr);
		}
	}
}