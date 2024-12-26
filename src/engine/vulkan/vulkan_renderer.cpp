#include "vulkan_renderer.h"
#include "vulkan_shader.h"
#include "vulkan.h"
#include "engine/application.h"

#include <VkBootstrap.h>
#include <backends/imgui_impl_vulkan.h>

namespace SDLarria 
{
    VulkanRenderer* VulkanRenderer::s_Instance;

	void VulkanRenderer::Initialize(SDL_Window* window, VkExtent2D windowSize) 
    {
        m_Instance.Initialize(window);

        m_BufferAllocator.Initialize(m_Instance);
        m_Swapchain.Initialize(m_Instance, windowSize);
        m_CommandPool.Initialize(m_Instance, 2);

        m_Framebuffer = VulkanImage(m_Instance.GetLogicalDevice(), m_BufferAllocator.GetAllocator(), windowSize);
        m_WindowSize = windowSize;

        ComputePipelineTest();
        s_Instance = this;
	}

	void VulkanRenderer::Draw() 
    {
        const auto& logicalDevice = m_Instance.GetLogicalDevice();
        const auto& currentBuffer = m_CommandPool.GetNextFrame();

        // begin render queue
        m_CommandPool.BeginCommandQueue();
        VkCommandBuffer cmd = currentBuffer.CommandBuffer;

        // prepare swapchain
        auto result = m_Swapchain.TryUpdateFrame(currentBuffer);
        if (result == VK_ERROR_OUT_OF_DATE_KHR) 
        {
            ResizeTest();
            return;
        }
        auto& swapchainImage = m_Swapchain.GetCurrentFrame();

        // render commands
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

        auto& image = swapchainImage.ImageData;
        VulkanImage::Transit(cmd, image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        // execute a copy from the draw image into the swapchain
        m_Framebuffer.Copy(cmd, image, m_Swapchain.GetSwapchainSize());

        // set swapchain image layout to Attachment Optimal so we can draw it
        VulkanImage::Transit(cmd, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

        auto colorAttachment = VkRenderingAttachmentInfo();
        colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        colorAttachment.pNext = nullptr;
        colorAttachment.imageView = swapchainImage.ImageViewData;
        colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

        VkRenderingInfo renderInfo = VkRenderingInfo();
        renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        renderInfo.pNext = nullptr;
        renderInfo.renderArea = VkRect2D{ VkOffset2D { 0, 0 }, m_Swapchain.GetSwapchainSize() };
        renderInfo.layerCount = 1;
        renderInfo.colorAttachmentCount = 1;
        renderInfo.pColorAttachments = &colorAttachment;
        renderInfo.pDepthAttachment = nullptr;
        renderInfo.pStencilAttachment = nullptr;

        vkCmdBeginRendering(cmd, &renderInfo);

        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);

        vkCmdEndRendering(cmd);

        // set swapchain image layout to Present so we can draw it
        VulkanImage::Transit(cmd, image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

        // end render queue
        m_CommandPool.EndCommandQueue();

        // render swapchain image
        result = m_Swapchain.TryShowSwapchain(currentBuffer);
        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            ResizeTest();
        }
        m_CurrentFrameIndex = std::max(m_CurrentFrameIndex + 1, m_Specs.FRAMES_IN_FLIGHT);
	}

    void VulkanRenderer::ResizeTest() 
    {
        auto queue = m_Instance.GetRenderQueue();
        auto device = m_Instance.GetLogicalDevice();
        auto window = Application::Get().GetWindow().GetNativeWindow();

        vkQueueWaitIdle(queue);

        SDL_GetWindowSizeInPixels(window, (int*)&m_WindowSize.width, (int*)&m_WindowSize.height);

        m_Swapchain.Destroy();
        m_Swapchain.Initialize(m_Instance, m_WindowSize);

        // recreate framebuffer
        m_BufferAllocator.DestroyVulkanImage(m_Framebuffer);
        m_Framebuffer = VulkanImage(device, m_BufferAllocator.GetAllocator(), m_WindowSize);

        // recreate descriptors
        VkDescriptorImageInfo imgInfo{};
        imgInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        imgInfo.imageView = m_Framebuffer.GetImageView();

        auto drawImageWrite = VkWriteDescriptorSet();
        drawImageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        drawImageWrite.pNext = nullptr;

        drawImageWrite.dstBinding = 0;
        drawImageWrite.dstSet = Test_drawImageDescriptors;
        drawImageWrite.descriptorCount = 1;
        drawImageWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        drawImageWrite.pImageInfo = &imgInfo;

        vkUpdateDescriptorSets(m_Instance.GetLogicalDevice(), 1, &drawImageWrite, 0, nullptr);
    }

	void VulkanRenderer::Shutdown() 
    {
        vkDeviceWaitIdle(m_Instance.GetLogicalDevice());

        m_CommandPool.Destroy();
        m_Swapchain.Destroy();

        m_DescriptorAllocator.Destroy();
        vkDestroyDescriptorSetLayout(m_Instance.GetLogicalDevice(), Test_drawImageDescriptorLayout, nullptr);
        vkDestroyPipelineLayout(m_Instance.GetLogicalDevice(), Test_gradientPipelineLayout, nullptr);
        vkDestroyPipeline(m_Instance.GetLogicalDevice(), Test_gradientPipeline, nullptr);

        m_BufferAllocator.DestroyVulkanImage(m_Framebuffer);
        m_BufferAllocator.Destroy();

        m_Instance.Destroy();
	}

    void VulkanRenderer::ComputePipelineTest()
    {
        //create a descriptor pool that will hold 10 sets with 1 image each
        std::vector<PoolSizeRatio> sizes =
        {
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1 }
        };

        m_DescriptorAllocator.Initialize(m_Instance, 10, sizes);

        auto builder = DescriptorLayoutBuilder();
        builder.AddBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
        Test_drawImageDescriptorLayout = builder.Build(m_Instance.GetLogicalDevice(), VK_SHADER_STAGE_COMPUTE_BIT);
        Test_drawImageDescriptors = m_DescriptorAllocator.AllocateSet(Test_drawImageDescriptorLayout);

        auto imgInfo = VkDescriptorImageInfo();
        imgInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        imgInfo.imageView = m_Framebuffer.GetImageView();

        auto drawImageWrite = VkWriteDescriptorSet();
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

        auto computePipelineCreateInfo = VkComputePipelineCreateInfo();
        computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        computePipelineCreateInfo.pNext = nullptr;
        computePipelineCreateInfo.layout = Test_gradientPipelineLayout;
        computePipelineCreateInfo.stage = testShader.GetRawShader();

        result = vkCreateComputePipelines(m_Instance.GetLogicalDevice(), VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, &Test_gradientPipeline);
        VULKAN_CHECK(result);

        testShader.Unload(m_Instance.GetLogicalDevice());
    }
}