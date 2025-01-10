#include "vulkan_command_queue.h"
#include "vulkan.h"
#include "vulkan_renderer.h"
#include "renderer/application.h"

namespace Moxel
{
	void VulkanCommandBuffer::Initialize(const int bufferCount)
	{
		const auto& toolset = Application::Get().GetContext();

		m_DeviceInstance = toolset.GetLogicalDevice();
		m_GraphicsQueue = toolset.GetRenderQueue();
		m_QueueFamily = toolset.GetQueueFamilyIndex();

		auto commandPoolInfo = VkCommandPoolCreateInfo();
		commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandPoolInfo.pNext = nullptr;
		commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		commandPoolInfo.queueFamilyIndex = m_QueueFamily;

		// allocate buffers
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

		// allocate fences and semaphores
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

		// create immediate buffer
		auto result = vkCreateCommandPool(m_DeviceInstance, &commandPoolInfo, nullptr, &m_ImmediatePool);
		VULKAN_CHECK(result);

		auto cmdAllocInfo = VkCommandBufferAllocateInfo();
		cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdAllocInfo.pNext = nullptr;
		cmdAllocInfo.commandPool = m_ImmediatePool;
		cmdAllocInfo.commandBufferCount = 1;
		cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

		result = vkAllocateCommandBuffers(m_DeviceInstance, &cmdAllocInfo, &m_ImmediateBuffer);
		VULKAN_CHECK(result);

		result = vkCreateFence(m_DeviceInstance, &fenceCreateInfo, nullptr, &m_ImmediateFence);
		VULKAN_CHECK(result);
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

		vkDestroyCommandPool(m_DeviceInstance, m_ImmediatePool, nullptr);
		vkDestroyFence(m_DeviceInstance, m_ImmediateFence, nullptr);
	}

	void VulkanCommandBuffer::BeginImmediateQueue() const
	{
		const auto device = Application::Get().GetContext().GetLogicalDevice();

		VULKAN_CHECK(vkResetFences(device, 1, &m_ImmediateFence));
		VULKAN_CHECK(vkResetCommandBuffer(m_ImmediateBuffer, 0));

		auto bufferBeginInfo = VkCommandBufferBeginInfo();
		bufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		bufferBeginInfo.pNext = nullptr;
		bufferBeginInfo.pInheritanceInfo = nullptr;
		bufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		VULKAN_CHECK(vkBeginCommandBuffer(m_ImmediateBuffer, &bufferBeginInfo));
	}

	void VulkanCommandBuffer::EndImmediateQueue() const
	{
		const auto device = Application::Get().GetContext().GetLogicalDevice();
		const auto queue = Application::Get().GetContext().GetRenderQueue();

		VULKAN_CHECK(vkEndCommandBuffer(m_ImmediateBuffer));

		auto bufferInfo = VkCommandBufferSubmitInfo();
		bufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
		bufferInfo.pNext = nullptr;
		bufferInfo.commandBuffer = m_ImmediateBuffer;
		bufferInfo.deviceMask = 0;

		auto submit = VkSubmitInfo2();
		submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
		submit.pNext = nullptr;
		//submit.waitSemaphoreInfoCount = 0;
		//submit.pWaitSemaphoreInfos = nullptr;
		//submit.signalSemaphoreInfoCount = 0;
		//submit.pSignalSemaphoreInfos = nullptr;
		submit.commandBufferInfoCount = 1;
		submit.pCommandBufferInfos = &bufferInfo;

		VULKAN_CHECK(vkQueueSubmit2(queue, 1, &submit, m_ImmediateFence));
		VULKAN_CHECK(vkWaitForFences(device, 1, &m_ImmediateFence, true, 9999999999));
	}

	void VulkanCommandBuffer::BeginCommandQueue() const
	{
		// prepare current command buffer
		const auto currentCommandBuffer = m_CurrentBuffer.CommandBuffer;
		auto result = vkResetCommandBuffer(currentCommandBuffer, 0);
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
		// end command buffer so we can read it
		auto result = vkEndCommandBuffer(m_CurrentBuffer.CommandBuffer);
		VULKAN_CHECK(result);

		// prepare the submission to the queue
		auto bufferInfo = VkCommandBufferSubmitInfo();
		bufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
		bufferInfo.pNext = nullptr;
		bufferInfo.commandBuffer = m_CurrentBuffer.CommandBuffer;
		bufferInfo.deviceMask = 0;

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
		submit.pCommandBufferInfos = &bufferInfo;

		// submit command buffer to the queue and execute it.
		result = vkQueueSubmit2(m_GraphicsQueue, 1, &submit, m_CurrentBuffer.RenderFence);
		VULKAN_CHECK(result);
	}

	CommandBufferData& VulkanCommandBuffer::GetNextFrame()
	{
		const auto frameIndex = VulkanRenderer::GetCurrentFrameIndex();
		m_CurrentBuffer = m_Buffers[frameIndex];

		return m_CurrentBuffer;
	}
}
