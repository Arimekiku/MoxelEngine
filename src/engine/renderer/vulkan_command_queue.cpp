#include "vulkan_command_queue.h"
#include "vulkan.h"
#include "vulkan_renderer.h"
#include "engine/application.h"

namespace Moxel
{
	void VulkanCommandBuffer::initialize(const int bufferCount)
	{
		const auto& toolset = Application::get().get_context();

		m_deviceInstance = toolset.get_logical_device();
		m_graphicsQueue = toolset.get_render_queue();
		m_queueFamily = toolset.get_queue_family_index();

		auto commandPoolInfo = VkCommandPoolCreateInfo();
		commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandPoolInfo.pNext = nullptr;
		commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		commandPoolInfo.queueFamilyIndex = m_queueFamily;

		// allocate buffers
		m_buffers.resize(bufferCount);
		for (auto& frame : m_buffers)
		{
			frame = CommandBufferData();

			auto result = vkCreateCommandPool(m_deviceInstance, &commandPoolInfo, nullptr, &frame.CommandPool);
			VULKAN_CHECK(result);

			auto cmdAllocInfo = VkCommandBufferAllocateInfo();
			cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			cmdAllocInfo.pNext = nullptr;
			cmdAllocInfo.commandPool = frame.CommandPool;
			cmdAllocInfo.commandBufferCount = 1;
			cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

			result = vkAllocateCommandBuffers(m_deviceInstance, &cmdAllocInfo, &frame.CommandBuffer);
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

		for (auto& frame : m_buffers)
		{
			auto result = vkCreateFence(m_deviceInstance, &fenceCreateInfo, nullptr, &frame.RenderFence);
			VULKAN_CHECK(result);

			result = vkCreateSemaphore(m_deviceInstance, &semaphoreCreateInfo, nullptr, &frame.SwapchainSemaphore);
			VULKAN_CHECK(result);

			result = vkCreateSemaphore(m_deviceInstance, &semaphoreCreateInfo, nullptr, &frame.RenderSemaphore);
			VULKAN_CHECK(result);
		}

		// create immediate buffer
		auto result = vkCreateCommandPool(m_deviceInstance, &commandPoolInfo, nullptr, &m_immediatePool);
		VULKAN_CHECK(result);

		auto allocationInfo = VkCommandBufferAllocateInfo();
		allocationInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocationInfo.pNext = nullptr;
		allocationInfo.commandPool = m_immediatePool;
		allocationInfo.commandBufferCount = 1;
		allocationInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

		result = vkAllocateCommandBuffers(m_deviceInstance, &allocationInfo, &m_immediateBuffer);
		VULKAN_CHECK(result);

		result = vkCreateFence(m_deviceInstance, &fenceCreateInfo, nullptr, &m_immediateFence);
		VULKAN_CHECK(result);
	}

	void VulkanCommandBuffer::destroy() const 
	{
		for (const auto& frame : m_buffers)
		{
			vkDestroyCommandPool(m_deviceInstance, frame.CommandPool, nullptr);
			vkDestroyFence(m_deviceInstance, frame.RenderFence, nullptr);
			vkDestroySemaphore(m_deviceInstance, frame.RenderSemaphore, nullptr);
			vkDestroySemaphore(m_deviceInstance, frame.SwapchainSemaphore, nullptr);
		}

		vkDestroyCommandPool(m_deviceInstance, m_immediatePool, nullptr);
		vkDestroyFence(m_deviceInstance, m_immediateFence, nullptr);
	}

	void VulkanCommandBuffer::begin_immediate_queue() const
	{
		const auto device = Application::get().get_context().get_logical_device();

		VULKAN_CHECK(vkResetFences(device, 1, &m_immediateFence));
		VULKAN_CHECK(vkResetCommandBuffer(m_immediateBuffer, 0));

		auto bufferBeginInfo = VkCommandBufferBeginInfo();
		bufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		bufferBeginInfo.pNext = nullptr;
		bufferBeginInfo.pInheritanceInfo = nullptr;
		bufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		VULKAN_CHECK(vkBeginCommandBuffer(m_immediateBuffer, &bufferBeginInfo));
	}

	void VulkanCommandBuffer::end_immediate_queue() const
	{
		const auto device = Application::get().get_context().get_logical_device();
		const auto queue = Application::get().get_context().get_render_queue();

		VULKAN_CHECK(vkEndCommandBuffer(m_immediateBuffer));

		auto bufferSubmitInfo = VkCommandBufferSubmitInfo();
		bufferSubmitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
		bufferSubmitInfo.pNext = nullptr;
		bufferSubmitInfo.commandBuffer = m_immediateBuffer;
		bufferSubmitInfo.deviceMask = 0;

		auto submit = VkSubmitInfo2();
		submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
		submit.pNext = nullptr;
		submit.commandBufferInfoCount = 1;
		submit.pCommandBufferInfos = &bufferSubmitInfo;

		VULKAN_CHECK(vkQueueSubmit2(queue, 1, &submit, m_immediateFence));
		VULKAN_CHECK(vkWaitForFences(device, 1, &m_immediateFence, true, 9999999999));
	}

	void VulkanCommandBuffer::begin_command_queue() const
	{
		// prepare current command buffer
		const auto currentCommandBuffer = m_currentBuffer.CommandBuffer;
		auto result = vkResetCommandBuffer(currentCommandBuffer, 0);
		VULKAN_CHECK(result);

		auto bufferBeginInfo = VkCommandBufferBeginInfo();
		bufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		bufferBeginInfo.pNext = nullptr;
		bufferBeginInfo.pInheritanceInfo = nullptr;
		bufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		result = vkBeginCommandBuffer(currentCommandBuffer, &bufferBeginInfo);
		VULKAN_CHECK(result);
	}

	void VulkanCommandBuffer::end_command_queue() const
	{
		// end command buffer so we can read it
		auto result = vkEndCommandBuffer(m_currentBuffer.CommandBuffer);
		VULKAN_CHECK(result);

		// prepare the submission to the queue
		auto bufferSubmitInfo = VkCommandBufferSubmitInfo();
		bufferSubmitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
		bufferSubmitInfo.pNext = nullptr;
		bufferSubmitInfo.commandBuffer = m_currentBuffer.CommandBuffer;
		bufferSubmitInfo.deviceMask = 0;

		auto waitInfo = VkSemaphoreSubmitInfo();
		waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
		waitInfo.pNext = nullptr;
		waitInfo.semaphore = m_currentBuffer.SwapchainSemaphore;
		waitInfo.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR;
		waitInfo.deviceIndex = 0;
		waitInfo.value = 1;

		auto signalInfo = VkSemaphoreSubmitInfo();
		signalInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
		signalInfo.pNext = nullptr;
		signalInfo.semaphore = m_currentBuffer.RenderSemaphore;
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
		submit.pCommandBufferInfos = &bufferSubmitInfo;

		// submit command buffer to the queue and execute it.
		result = vkQueueSubmit2(m_graphicsQueue, 1, &submit, m_currentBuffer.RenderFence);
		VULKAN_CHECK(result);
	}

	CommandBufferData& VulkanCommandBuffer::get_next_frame()
	{
		const auto frameIndex = VulkanRenderer::get_current_frame_index();
		m_currentBuffer = m_buffers[frameIndex];

		return m_currentBuffer;
	}
}
