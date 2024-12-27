#include "vulkan_command_queue.h"
#include "vulkan_renderer.h"
#include "vulkan.h"

namespace SDLarria 
{
	void VulkanCommandBuffer::Initialize(const int bufferCount)
	{
		const auto& toolset = VulkanRenderer::Get().GetContext();

		m_DeviceInstance = toolset.GetLogicalDevice();
		m_GraphicsQueue = toolset.GetRenderQueue();
		m_QueueFamily = toolset.GetQueueFamilyIndex();

		auto commandPoolInfo = VkCommandPoolCreateInfo();
		commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandPoolInfo.pNext = nullptr;
		commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		commandPoolInfo.queueFamilyIndex = m_QueueFamily;

		// Allocate buffers
		m_Buffers.resize(bufferCount);
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

	void VulkanCommandBuffer::BeginCommandQueue() const
	{
		// update fences
		const auto device = VulkanRenderer::Get().GetContext().GetLogicalDevice();

		auto result = vkWaitForFences(device, 1, &m_CurrentBuffer.RenderFence, true, 1000000000);
		VULKAN_CHECK(result);

		result = vkResetFences(device, 1, &m_CurrentBuffer.RenderFence);
		VULKAN_CHECK(result);

		// prepare current command buffer
		const auto currentCommandBuffer = m_CurrentBuffer.CommandBuffer;
		result = vkResetCommandBuffer(currentCommandBuffer, 0);
		VULKAN_CHECK(result);

		auto cmdBeginInfo = VkCommandBufferBeginInfo();
		cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		cmdBeginInfo.pNext = nullptr;
		cmdBeginInfo.pInheritanceInfo = nullptr;
		cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		result = vkBeginCommandBuffer(currentCommandBuffer, &cmdBeginInfo);
		VULKAN_CHECK(result);
	}

	void VulkanCommandBuffer::EndCommandQueue() const
	{
		// we can no longer add commands, but it can now be executed
		auto result = vkEndCommandBuffer(m_CurrentBuffer.CommandBuffer);
		VULKAN_CHECK(result);

		// prepare the submission to the queue.
		auto cmdInfo = VkCommandBufferSubmitInfo();
		cmdInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
		cmdInfo.pNext = nullptr;
		cmdInfo.commandBuffer = m_CurrentBuffer.CommandBuffer;
		cmdInfo.deviceMask = 0;

		auto waitInfo = VkSemaphoreSubmitInfo();
		waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
		waitInfo.pNext = nullptr;
		waitInfo.semaphore = m_CurrentBuffer.SwapchainSemaphore;
		waitInfo.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR;
		waitInfo.deviceIndex = 0;
		waitInfo.value = 1;

		auto signalInfo = VkSemaphoreSubmitInfo();
		signalInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
		signalInfo.pNext = nullptr;
		signalInfo.semaphore = m_CurrentBuffer.RenderSemaphore;
		signalInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;
		signalInfo.deviceIndex = 0;
		signalInfo.value = 1;

		auto submit = VkSubmitInfo2();
		submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
		submit.pNext = nullptr;
		submit.waitSemaphoreInfoCount = 1;
		submit.pWaitSemaphoreInfos = &waitInfo;
		submit.signalSemaphoreInfoCount = 1;
		submit.pSignalSemaphoreInfos = &signalInfo;
		submit.commandBufferInfoCount = 1;
		submit.pCommandBufferInfos = &cmdInfo;

		// submit command buffer to the queue and execute it.
		result = vkQueueSubmit2(m_GraphicsQueue, 1, &submit, m_CurrentBuffer.RenderFence);
		VULKAN_CHECK(result);
	}

	CommandBufferData& VulkanCommandBuffer::GetNextFrame()
	{
		const auto frameIndex = VulkanRenderer::Get().GetCurrentFrameIndex();
		m_CurrentBuffer = m_Buffers[frameIndex];

		return m_CurrentBuffer;
	}
}