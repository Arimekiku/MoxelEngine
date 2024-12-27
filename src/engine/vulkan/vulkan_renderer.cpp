#include "vulkan_renderer.h"
#include "vulkan_shader.h"
#include "vulkan.h"
#include "engine/application.h"

#include <VkBootstrap.h>
#include <backends/imgui_impl_vulkan.h>

namespace SDLarria 
{
    VulkanRenderer* VulkanRenderer::s_Instance;

	void VulkanRenderer::Initialize(SDL_Window* window, const VkExtent2D& windowSize)
    {
        s_Instance = this;

        m_Context.Initialize(window);

        m_BufferAllocator.Initialize();
        m_Swapchain.Initialize(windowSize);
        m_CommandPool.Initialize(m_Specs.FRAMES_IN_FLIGHT);

        m_Framebuffer = VulkanImage(m_Context.GetLogicalDevice(), m_BufferAllocator.GetAllocator(), windowSize);

	    m_Swapchain.QueueResize([&]
	    {
	        const auto device = m_Context.GetLogicalDevice();
            const auto& framebufferSize = Application::Get().GetWindow().GetWindowSize();

            // recreate framebuffer
            m_BufferAllocator.DestroyVulkanImage(m_Framebuffer);
            m_Framebuffer = VulkanImage(device, m_BufferAllocator.GetAllocator(), framebufferSize);

            // recreate shaders
            m_GradientShader.Reload();
	    });

        ComputePipelineTest();
	}

	void VulkanRenderer::Draw() 
    {
        const auto& currentBuffer = m_CommandPool.GetNextFrame();

        // begin render queue
        m_CommandPool.BeginCommandQueue();
        VkCommandBuffer cmd = currentBuffer.CommandBuffer;

        // prepare swapchain
        m_Swapchain.UpdateFrame(currentBuffer);
        const auto& swapchainImage = m_Swapchain.GetCurrentFrame();

        // render commands
        // transit framebuffer into writeable mode
        VulkanImage::Transit(m_Framebuffer.GetRawImage(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

        // write gradient into framebuffer
		const auto& imageSize = m_Framebuffer.GetImageSize();

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_GradientShader.GetShaderPipeline());
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_GradientShader.GetShaderPipelineLayout(), 0, 1, m_GradientShader.GetDescriptors(), 0, nullptr);
        vkCmdDispatch(cmd, std::ceil(imageSize.width / 16.0), std::ceil(imageSize.height / 16.0), 1);

        // transit framebuffer into transfer source mode
        VulkanImage::Transit(m_Framebuffer.GetRawImage(), VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

        // transit current swapchain frame into transfer destination mode
        const auto& image = swapchainImage.ImageData;
        VulkanImage::Transit(image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        // write framebuffer into current swapchain image
        m_Framebuffer.CopyRaw(image, m_Swapchain.GetSwapchainSize());

        // set current frame into rendering mode
        VulkanImage::Transit(image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

        auto colorAttachment = VkRenderingAttachmentInfo();
        colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        colorAttachment.imageView = swapchainImage.ImageViewData;
        colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

        auto renderInfo = VkRenderingInfo();
        renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        renderInfo.renderArea = VkRect2D(VkOffset2D(0, 0), m_Swapchain.GetSwapchainSize());
        renderInfo.layerCount = 1;
        renderInfo.colorAttachmentCount = 1;
        renderInfo.pColorAttachments = &colorAttachment;

        // render imgui
        vkCmdBeginRendering(cmd, &renderInfo);

        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);

        vkCmdEndRendering(cmd);

        // set current mode into present so we can draw it on screen
        VulkanImage::Transit(image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

        // end render queue
        m_CommandPool.EndCommandQueue();

        // render swapchain image
		m_Swapchain.ShowSwapchain(currentBuffer);
        m_CurrentFrameIndex = std::max(m_CurrentFrameIndex + 1, m_Specs.FRAMES_IN_FLIGHT);
	}

	void VulkanRenderer::Shutdown() 
    {
        const auto device = m_Context.GetLogicalDevice();
        vkDeviceWaitIdle(device);

        m_CommandPool.Destroy();
        m_Swapchain.Destroy();

        m_DescriptorAllocator.Destroy();
        m_GradientShader.Destroy();

        m_BufferAllocator.DestroyVulkanImage(m_Framebuffer);
        m_BufferAllocator.Destroy();

        m_Context.Destroy();
	}

    void VulkanRenderer::ComputePipelineTest()
    {
        //create a descriptor pool that will hold 10 sets with 1 image each
        std::vector<PoolSizeRatio> sizes =
        {
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1 }
        };

        m_DescriptorAllocator.Initialize(10, sizes);

        m_GradientShader = VulkanShader(RESOURCES_PATH "test.comp.spv", m_DescriptorAllocator);
    }
}