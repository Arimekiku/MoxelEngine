#include "vulkan_renderer.h"
#include "vulkan_shader.h"
#include "engine/application.h"
#include "vulkan.h"

#include <VkBootstrap.h>
#include <backends/imgui_impl_vulkan.h>
#include <memory>

namespace SDLarria 
{
    VulkanRenderer* VulkanRenderer::s_Instance;

	void VulkanRenderer::Initialize(SDL_Window* window, const VkExtent2D& windowSize)
    {
        s_Instance = this;

		// initialize vulkan
        m_Context.Initialize(window);
        m_BufferAllocator.Initialize();
        m_Swapchain.Initialize(windowSize);
        m_CommandPool.Initialize(m_Specs.FRAMES_IN_FLIGHT);

        m_Framebuffer = std::make_shared<VulkanImage>(m_BufferAllocator.GetAllocator(), windowSize);

		// setup shaders and pipelines
		std::vector<PoolSizeRatio> sizes =
		{
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1 }
		};

        m_DescriptorAllocator.Initialize(10, sizes);
		m_GradientShader = std::make_shared<VulkanShader>(RESOURCES_PATH "sky.comp.spv", m_DescriptorAllocator, ShaderType::COMPUTE);

		const auto fragment = std::make_shared<VulkanShader>(RESOURCES_PATH "triangle.frag.spv", m_DescriptorAllocator, ShaderType::FRAGMENT);
		const auto vertex = std::make_shared<VulkanShader>(RESOURCES_PATH "triangle.vert.spv", m_DescriptorAllocator, ShaderType::VERTEX);

		const auto graphicSpecs = VulkanGraphicsPipelineSpecs
		{
			fragment,
			vertex,
			m_Framebuffer,

			VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			VK_POLYGON_MODE_FILL,
			VK_CULL_MODE_NONE,
			VK_FRONT_FACE_CLOCKWISE
		};
		m_GraphicsPipeline = VulkanGraphicsPipeline(graphicSpecs);

		auto computePushConstants = VkPushConstantRange();
		computePushConstants.offset = 0;
		computePushConstants.size = sizeof(ComputePushConstants_TEST);
		computePushConstants.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

		const auto computeSpecs = VulkanComputePipelineSpecs
		{
			m_GradientShader,
			m_Framebuffer,

			computePushConstants
		};
		m_GradientPipeline = VulkanComputePipeline(computeSpecs);

		auto& pushConst = m_GradientShader->GetPushConstants();
		pushConst.data1 = glm::vec4(0.1, 0.2, 0.4 ,0.97);

		const auto triangleVertexShader = std::make_shared<VulkanShader>(RESOURCES_PATH "triangle_meshed.vert.spv", m_DescriptorAllocator, ShaderType::VERTEX);

		VkPushConstantRange bufferRange{};
		bufferRange.offset = 0;
		bufferRange.size = sizeof(GPUDrawPushConstants_TEST);
		bufferRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		const auto meshedSpecs = VulkanGraphicsPipelineSpecs
		{
			fragment,
			triangleVertexShader,
			m_Framebuffer,

			VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			VK_POLYGON_MODE_FILL,
			VK_CULL_MODE_NONE,
			VK_FRONT_FACE_CLOCKWISE,
			bufferRange
		};
		m_MeshedPipeline = VulkanGraphicsPipeline(meshedSpecs);

		// setup vertex array
		std::vector<Vertex> rect_vertices;
		rect_vertices.resize(4);

		rect_vertices[0].position = {0.5,-0.5, 0};
		rect_vertices[1].position = {0.5,0.5, 0};
		rect_vertices[2].position = {-0.5,-0.5, 0};
		rect_vertices[3].position = {-0.5,0.5, 0};

		rect_vertices[0].color = {0,0, 0,1};
		rect_vertices[1].color = { 0.5,0.5,0.5 ,1};
		rect_vertices[2].color = { 1,0, 0,1 };
		rect_vertices[3].color = { 0,1, 0,1 };

		std::vector<uint32_t> rect_indices;
		rect_indices.resize(6);

		rect_indices[0] = 0;
		rect_indices[1] = 1;
		rect_indices[2] = 2;

		rect_indices[3] = 2;
		rect_indices[4] = 1;
		rect_indices[5] = 3;

		m_Rectangle = VulkanVertexArray(m_BufferAllocator.GetAllocator(), rect_indices, rect_vertices);

		fragment->Release();
		vertex->Release();
		triangleVertexShader->Release();

		// setup resize function
	    m_Swapchain.QueueResize([&]
	    {
            const auto& framebufferSize = Application::Get().GetWindow().GetWindowSize();

            // recreate framebuffer
            m_BufferAllocator.DestroyVulkanImage(m_Framebuffer);
            m_Framebuffer = std::make_shared<VulkanImage>(m_BufferAllocator.GetAllocator(), framebufferSize);

            // recreate pipelines
	    	m_GradientPipeline.Reload();
	    });
	}

	void VulkanRenderer::ImmediateSubmit(std::function<void(VkCommandBuffer freeBuffer)>&& function) const
	{
		const auto cmd = m_CommandPool.GetImmediateBuffer();
		m_CommandPool.BeginImmediateQueue();

		function(cmd);

		m_CommandPool.EndImmediateQueue();
	}

	void VulkanRenderer::Draw() 
    {
        const auto& currentBuffer = m_CommandPool.GetNextFrame();

        // begin render queue
        m_CommandPool.BeginCommandQueue();
        const auto buffer = currentBuffer.CommandBuffer;

        // prepare swapchain
        m_Swapchain.UpdateFrame(currentBuffer);
        const auto& swapchainImage = m_Swapchain.GetCurrentFrame();

        // render commands
        // transit framebuffer into writeable mode
        VulkanImage::Transit(m_Framebuffer->GetRawImage(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

        // write gradient into framebuffer
		const auto& imageSize = m_Framebuffer->GetImageSize();
		const VkDescriptorSet& pSet = m_GradientShader->GetDescriptors();

        vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_GradientPipeline.GetPipeline());
        vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_GradientPipeline.GetLayout(), 0, 1, &pSet, 0, nullptr);

		const auto& pc = m_GradientShader->GetPushConstants();
		vkCmdPushConstants(buffer, m_GradientPipeline.GetLayout(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(ComputePushConstants_TEST), &pc);

        vkCmdDispatch(buffer, std::ceil(imageSize.width / 16.0), std::ceil(imageSize.height / 16.0), 1);

		VulkanImage::Transit(m_Framebuffer->GetRawImage(), VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

		// setup render infos
		auto colorAttachment = VkRenderingAttachmentInfo();
		colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		colorAttachment.imageView = m_Framebuffer->GetImageView();
		colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

		auto renderInfo = VkRenderingInfo();
		renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
		renderInfo.renderArea = VkRect2D(VkOffset2D(0, 0), m_Swapchain.GetSwapchainSize());
		renderInfo.layerCount = 1;
		renderInfo.colorAttachmentCount = 1;
		renderInfo.pColorAttachments = &colorAttachment;

		// draw geometry
		vkCmdBeginRendering(buffer, &renderInfo);

		vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline.GetPipeline());

		//set dynamic viewport and scissor
		const auto framebufferSize = m_Framebuffer->GetImageSize();

		auto viewport = VkViewport();
		viewport.x = 0;
		viewport.y = 0;
		viewport.width = framebufferSize.width;
		viewport.height = framebufferSize.height;
		viewport.minDepth = 0.f;
		viewport.maxDepth = 1.f;

		vkCmdSetViewport(buffer, 0, 1, &viewport);

		auto scissor = VkRect2D();
		scissor.offset.x = 0;
		scissor.offset.y = 0;
		scissor.extent.width = framebufferSize.width;
		scissor.extent.height = framebufferSize.height;

		vkCmdSetScissor(buffer, 0, 1, &scissor);

		//launch a draw command to draw 3 vertices
		vkCmdDraw(buffer, 3, 1, 0, 0);

		vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_MeshedPipeline.GetPipeline());

		GPUDrawPushConstants_TEST push_constants{};
		push_constants.worldMatrix = glm::mat4{ 1.f };
		push_constants.vertexBuffer = m_Rectangle.GetRenderingID();

		vkCmdPushConstants(buffer, m_MeshedPipeline.GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPUDrawPushConstants_TEST), &push_constants);
		vkCmdBindIndexBuffer(buffer, m_Rectangle.GetIndexBuffer().Buffer, 0, VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexed(buffer, 6, 1, 0, 0, 0);

		vkCmdEndRendering(buffer);

        // transit framebuffer into transfer source mode
        VulkanImage::Transit(m_Framebuffer->GetRawImage(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

        // transit current swapchain frame into transfer destination mode
        const auto& image = swapchainImage.ImageData;
        VulkanImage::Transit(image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        // write framebuffer into current swapchain image
        m_Framebuffer->CopyRaw(image, m_Swapchain.GetSwapchainSize());

        // set current frame into rendering mode
        VulkanImage::Transit(image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

		// setup render infos
		colorAttachment = VkRenderingAttachmentInfo();
        colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        colorAttachment.imageView = swapchainImage.ImageViewData;
        colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

		renderInfo = VkRenderingInfo();
        renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        renderInfo.renderArea = VkRect2D(VkOffset2D(0, 0), m_Swapchain.GetSwapchainSize());
        renderInfo.layerCount = 1;
        renderInfo.colorAttachmentCount = 1;
        renderInfo.pColorAttachments = &colorAttachment;

		// draw imgui
        vkCmdBeginRendering(buffer, &renderInfo);

        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), buffer);

        vkCmdEndRendering(buffer);

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
		m_GradientPipeline.Destroy();
		m_GraphicsPipeline.Destroy();

        m_BufferAllocator.DestroyVulkanImage(m_Framebuffer);
		m_BufferAllocator.DestroyBuffer(m_Rectangle.GetIndexBuffer());
		m_BufferAllocator.DestroyBuffer(m_Rectangle.GetVertexBuffer());
        m_BufferAllocator.Destroy();

        m_Context.Destroy();
	}
}