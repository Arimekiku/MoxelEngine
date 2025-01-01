#include "vulkan_renderer.h"
#include "vulkan_shader.h"
#include "engine/application.h"
#include "vulkan.h"

#include <VkBootstrap.h>
#include <backends/imgui_impl_vulkan.h>
#include <memory>

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>

#include "glm/gtx/quaternion.hpp"

namespace SDLarria 
{
    VulkanRenderer* VulkanRenderer::s_Instance;

	struct UniformBufferObject_TEST
	{
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
	};

	void VulkanRenderer::Initialize(SDL_Window* window, const VkExtent2D& windowSize)
    {
        s_Instance = this;

		// initialize vulkan
        m_Context.Initialize(window);
        VulkanAllocator::Initialize();
        m_Swapchain.Initialize(windowSize);
        m_CommandPool.Initialize(m_Specs.FRAMES_IN_FLIGHT);

        m_Framebuffer = std::make_shared<VulkanImage>(windowSize);

		// setup shaders and pipelines
		m_Uniforms.resize(m_Specs.FRAMES_IN_FLIGHT);
		for (auto& uniform : m_Uniforms)
		{
			uniform = std::make_shared<VulkanBufferUniform>(sizeof(UniformBufferObject_TEST));
		}

		m_GlobalDescriptorPool = VulkanDescriptorPool::Builder()
			.SetMaxSets(m_Specs.FRAMES_IN_FLIGHT)
			.AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, m_Specs.FRAMES_IN_FLIGHT)
			.Build();

		const auto globalSetLayout = VulkanDescriptorSetLayout::Builder()
			.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
			.Build();

		m_GlobalSets = std::vector<VkDescriptorSet>(m_Specs.FRAMES_IN_FLIGHT);
		for (int i = 0; i < m_GlobalSets.size(); ++i)
		{
			const auto& bufferInfo = m_Uniforms[i]->GetDescriptorInfo();

			DescriptorWriter(*globalSetLayout, *m_GlobalDescriptorPool)
				.WriteBuffer(0, bufferInfo)
				.Build(m_GlobalSets[i]);
		}

		const auto fragment = std::make_shared<VulkanShader>(RESOURCES_PATH "triangle.frag.spv", ShaderType::FRAGMENT);
		const auto triangleVertexShader = std::make_shared<VulkanShader>(RESOURCES_PATH "triangle_meshed.vert.spv", ShaderType::VERTEX);

		const auto meshedSpecs = VulkanGraphicsPipelineSpecs
		{
			fragment,
			triangleVertexShader,
			m_Framebuffer,

			VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			VK_POLYGON_MODE_FILL,
			VK_CULL_MODE_NONE,
			VK_FRONT_FACE_CLOCKWISE,
		};
		m_MeshedPipeline = VulkanGraphicsPipeline(meshedSpecs, globalSetLayout->GetDescriptorSetLayout());

		// setup vertex array
		std::vector<Vertex> rect_vertices;
		rect_vertices.resize(4);

		rect_vertices[0].Position = { 0.5, -0.5, 0 };
		rect_vertices[1].Position = { 0.5, 0.5, 0 };
		rect_vertices[2].Position = { -0.5, -0.5, 0 };
		rect_vertices[3].Position = { -0.5, 0.5, 0 };

		rect_vertices[0].Color = { 0, 0, 0 };
		rect_vertices[1].Color = { 0.5, 0.5, 0.5 };
		rect_vertices[2].Color = { 1, 0, 0 };
		rect_vertices[3].Color = { 0, 1, 0 };

		std::vector<uint32_t> rect_indices;
		rect_indices.resize(6);

		rect_indices[0] = 0;
		rect_indices[1] = 1;
		rect_indices[2] = 2;

		rect_indices[3] = 2;
		rect_indices[4] = 1;
		rect_indices[5] = 3;

		m_Rectangle = VulkanVertexArray(rect_indices, rect_vertices);

		m_ShaderLibrary.Add(fragment);
		fragment->Release();
		m_ShaderLibrary.Add(triangleVertexShader);
		triangleVertexShader->Release();

		// setup resize function
	    m_Swapchain.QueueResize([&]
	    {
            const auto& framebufferSize = Application::Get().GetWindow().GetWindowSize();

            // recreate framebuffer
            VulkanAllocator::DestroyVulkanImage(m_Framebuffer);
            m_Framebuffer = std::make_shared<VulkanImage>(framebufferSize);

            // TODO: recreate pipelines
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
        // transit framebuffer into writeable mod
		VulkanImage::Transit(m_Framebuffer->GetRawImage(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

		// setup render infos
		auto colorAttachment = VkRenderingAttachmentInfo();
		colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		colorAttachment.imageView = m_Framebuffer->GetImageView();
		colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

		auto renderInfo = VkRenderingInfo();
		renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
		renderInfo.renderArea = VkRect2D(VkOffset2D(0, 0), m_Swapchain.GetSwapchainSize());
		renderInfo.layerCount = 1;
		renderInfo.colorAttachmentCount = 1;
		renderInfo.pColorAttachments = &colorAttachment;

		// draw geometry
		vkCmdBeginRendering(buffer, &renderInfo);

		//set dynamic viewport and scissor
		const auto [width, height] = m_Swapchain.GetSwapchainSize();

		auto viewport = VkViewport();
		viewport.x = 0;
		viewport.y = 0;
		viewport.width = width;
		viewport.height = height;
		viewport.minDepth = 0.f;
		viewport.maxDepth = 1.f;
		vkCmdSetViewport(buffer, 0, 1, &viewport);

		auto scissor = VkRect2D();
		scissor.offset.x = 0;
		scissor.offset.y = 0;
		scissor.extent.width = width;
		scissor.extent.height = height;
		vkCmdSetScissor(buffer, 0, 1, &scissor);

		// update uniform data
		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float>(currentTime - startTime).count();

		UniformBufferObject_TEST ubo{};
		ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

		ubo.proj[1][1] *= -1;

		m_Uniforms[m_CurrentFrameIndex]->WriteData(&ubo, sizeof(ubo));

		//launch a draw command to draw vertices
		vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_MeshedPipeline.GetPipeline());

		VkBuffer vertexBuffers[] = { m_Rectangle.GetVertexBuffer().Buffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(buffer, 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(buffer, m_Rectangle.GetIndexBuffer().Buffer, 0, VK_INDEX_TYPE_UINT32);

		auto set = m_GlobalSets[m_CurrentFrameIndex];
		vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_MeshedPipeline.GetPipelineLayout(), 0, 1, &set, 0, nullptr);
		vkCmdDrawIndexed(buffer, 6, 1, 0, 0, 0);

		vkCmdEndRendering(buffer);

		// copy framebuffer into swapchain
		VulkanImage::Transit(m_Framebuffer->GetRawImage(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		VulkanImage::Transit(swapchainImage.ImageData, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		m_Framebuffer->CopyRaw(swapchainImage.ImageData, m_Swapchain.GetSwapchainSize());
		VulkanImage::Transit(swapchainImage.ImageData, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

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
        VulkanImage::Transit(swapchainImage.ImageData, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

        // end render queue
        m_CommandPool.EndCommandQueue();

        // render swapchain image
		m_Swapchain.ShowSwapchain(currentBuffer);
        m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % m_Specs.FRAMES_IN_FLIGHT;
	}

	void VulkanRenderer::Shutdown() 
    {
        const auto device = m_Context.GetLogicalDevice();
        vkDeviceWaitIdle(device);

        m_CommandPool.Destroy();
        m_Swapchain.Destroy();

		m_MeshedPipeline.Destroy();
		m_ShaderLibrary.Destroy();

        VulkanAllocator::DestroyVulkanImage(m_Framebuffer);
		VulkanAllocator::DestroyBuffer(m_Rectangle.GetIndexBuffer());
		VulkanAllocator::DestroyBuffer(m_Rectangle.GetVertexBuffer());

		m_GlobalDescriptorPool = nullptr;
		m_GlobalSets.clear();
		m_Uniforms.clear();

        VulkanAllocator::Destroy();

        m_Context.Destroy();
	}
}
