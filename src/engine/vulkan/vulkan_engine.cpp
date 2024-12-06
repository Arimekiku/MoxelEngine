#include "vulkan_engine.h"
#include "vulkan.h"

#include <cmath>
#include <VkBootstrap.h>

namespace SDLarria 
{
	void VulkanEngine::Initialize(SDL_Window* window, VkExtent2D windowSize) 
    {
        m_Instance.Initialize(window);

        m_BufferAllocator.Initialize(m_Instance);
        m_Swapchain.Initialize(m_Instance, windowSize);
        m_CommandPool.Initialize(m_Instance);

        m_Framebuffer = VulkanImage(m_Instance.GetLogicalDevice(), m_BufferAllocator.GetAllocator(), windowSize);
        m_WindowSize = windowSize;

        //create a descriptor pool that will hold 10 sets with 1 image each
        std::vector<PoolSizeRatio> sizes =
        {
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1 }
        };

        m_DescriptorAllocator.Initialize(m_Instance, 10, sizes);

        DescriptorLayoutBuilder builder;
        builder.AddBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
        Test_drawImageDescriptorLayout = builder.Build(m_Instance.GetLogicalDevice(), VK_SHADER_STAGE_COMPUTE_BIT);
        Test_drawImageDescriptors = m_DescriptorAllocator.AllocateSet(Test_drawImageDescriptorLayout);

        VkDescriptorImageInfo imgInfo{};
        imgInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        imgInfo.imageView = m_Framebuffer.GetImageView();

        VkWriteDescriptorSet drawImageWrite = {};
        drawImageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        drawImageWrite.pNext = nullptr;

        drawImageWrite.dstBinding = 0;
        drawImageWrite.dstSet = Test_drawImageDescriptors;
        drawImageWrite.descriptorCount = 1;
        drawImageWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        drawImageWrite.pImageInfo = &imgInfo;

        vkUpdateDescriptorSets(m_Instance.GetLogicalDevice(), 1, &drawImageWrite, 0, nullptr);

        auto computeLayout = VkPipelineLayoutCreateInfo();
        computeLayout.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        computeLayout.pNext = nullptr;
        computeLayout.pSetLayouts = &Test_drawImageDescriptorLayout;
        computeLayout.setLayoutCount = 1;

        auto result = vkCreatePipelineLayout(m_Instance.GetLogicalDevice(), &computeLayout, nullptr, &Test_gradientPipelineLayout);
        VULKAN_CHECK(result);

        VulkanShader testShader = VulkanShader(RESOURCES_PATH "test.comp.spv", m_Instance.GetLogicalDevice());
        VkShaderModule computeDrawShader = testShader.GetRawShader();

        auto stageinfo = VkPipelineShaderStageCreateInfo();
        stageinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stageinfo.pNext = nullptr;
        stageinfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        stageinfo.module = computeDrawShader;
        stageinfo.pName = "main";

        auto computePipelineCreateInfo = VkComputePipelineCreateInfo();
        computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        computePipelineCreateInfo.pNext = nullptr;
        computePipelineCreateInfo.layout = Test_gradientPipelineLayout;
        computePipelineCreateInfo.stage = stageinfo;

        result = vkCreateComputePipelines(m_Instance.GetLogicalDevice(), VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, &Test_gradientPipeline);
        VULKAN_CHECK(result);

        vkDestroyShaderModule(m_Instance.GetLogicalDevice(), computeDrawShader, nullptr);

	}

	 void VulkanEngine::Draw() 
     {
        const auto& logicalDevice = m_Instance.GetLogicalDevice();

        // Update fences
        auto result = vkWaitForFences(logicalDevice, 1, &m_CommandPool.GetLastFrame().RenderFence, true, 1000000000);
        VULKAN_CHECK(result);

        result = vkResetFences(logicalDevice, 1, &m_CommandPool.GetLastFrame().RenderFence);
        VULKAN_CHECK(result);

        uint32_t swapchainImageIndex;
        const auto& lastFrame = m_CommandPool.GetLastFrame();
        result = vkAcquireNextImageKHR(logicalDevice, m_Swapchain.m_SwapchainInstance, 1000000000, lastFrame.SwapchainSemaphore, nullptr, &swapchainImageIndex);
        VULKAN_CHECK(result);

        VkCommandBuffer cmd = lastFrame.CommandBuffer;

        // Render commands
        result = vkResetCommandBuffer(cmd, 0);
        VULKAN_CHECK(result);

        auto cmdBeginInfo = VkCommandBufferBeginInfo();
        cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        cmdBeginInfo.pNext = nullptr;
        cmdBeginInfo.pInheritanceInfo = nullptr;
        cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        result = vkBeginCommandBuffer(cmd, &cmdBeginInfo);
        VULKAN_CHECK(result);

        // make the swapchain image into writeable mode before rendering
        VulkanImage::Transit(cmd, m_Framebuffer.GetRawImage(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

        // bind the gradient drawing compute pipeline
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, Test_gradientPipeline);

        // bind the descriptor set containing the draw image for the compute pipeline
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, Test_gradientPipelineLayout, 0, 1, &Test_drawImageDescriptors, 0, nullptr);

        // execute the compute pipeline dispatch. We are using 16x16 workgroup size so we need to divide by it
        vkCmdDispatch(cmd, std::ceil(m_WindowSize.width / 16.0), std::ceil(m_WindowSize.height / 16.0), 1);

        //transition the draw image and the swapchain image into their correct transfer layouts
        VulkanImage::Transit(cmd, m_Framebuffer.GetRawImage(), VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

        auto& image = m_Swapchain.m_Images[swapchainImageIndex];
        VulkanImage::Transit(cmd, image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        // execute a copy from the draw image into the swapchain
        m_Framebuffer.Copy(cmd, image, m_Swapchain.GetSwapchainSize());

        // set swapchain image layout to Present so we can show it on the screen
        VulkanImage::Transit(cmd, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

        // finalize the command buffer (we can no longer add commands, but it can now be executed)
        result = vkEndCommandBuffer(cmd);
        VULKAN_CHECK(result);

        // prepare the submission to the queue.
        // we want to wait on the _presentSemaphore, as that semaphore is signaled when the swapchain is ready
        // we will signal the _renderSemaphore, to signal that rendering has finished
        auto cmdInfo = VkCommandBufferSubmitInfo();
        cmdInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
        cmdInfo.pNext = nullptr;
        cmdInfo.commandBuffer = cmd;
        cmdInfo.deviceMask = 0;

        auto waitInfo = VkSemaphoreSubmitInfo();
        waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        waitInfo.pNext = nullptr;
        waitInfo.semaphore = lastFrame.SwapchainSemaphore;
        waitInfo.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR;
        waitInfo.deviceIndex = 0;
        waitInfo.value = 1;

        auto signalInfo = VkSemaphoreSubmitInfo();
        signalInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        signalInfo.pNext = nullptr;
        signalInfo.semaphore = lastFrame.RenderSemaphore;
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
        // Fence will now block until the graphic commands finish execution
        result = vkQueueSubmit2(m_CommandPool.m_GraphicsQueue, 1, &submit, lastFrame.RenderFence);
        VULKAN_CHECK(result);

        // prepare present
        // this will put the image we just rendered to into the visible window.
        // we want to wait on the _renderSemaphore for that,
        // as its necessary that drawing commands have finished before the image is displayed to the user
        auto presentInfo = VkPresentInfoKHR();
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.pNext = nullptr;
        presentInfo.pSwapchains = &m_Swapchain.m_SwapchainInstance;
        presentInfo.swapchainCount = 1;

        presentInfo.pWaitSemaphores = &lastFrame.RenderSemaphore;
        presentInfo.waitSemaphoreCount = 1;

        presentInfo.pImageIndices = &swapchainImageIndex;

        result = vkQueuePresentKHR(m_CommandPool.m_GraphicsQueue, &presentInfo);
        VULKAN_CHECK(result);

        //increase the number of frames drawn
        m_CommandPool.m_CurrentFrame++;
	 }

	void VulkanEngine::Shutdown() 
    {
        m_Swapchain.Destroy();
        m_CommandPool.Destroy();
        m_BufferAllocator.DestroyVulkanImage(m_Framebuffer);
        m_BufferAllocator.Destroy();

        m_DescriptorAllocator.Destroy();
        vkDestroyDescriptorSetLayout(m_Instance.GetLogicalDevice(), Test_drawImageDescriptorLayout, nullptr);
        vkDestroyPipelineLayout(m_Instance.GetLogicalDevice(), Test_gradientPipelineLayout, nullptr);
        vkDestroyPipeline(m_Instance.GetLogicalDevice(), Test_gradientPipeline, nullptr);

        m_Instance.Destroy();
	}
}