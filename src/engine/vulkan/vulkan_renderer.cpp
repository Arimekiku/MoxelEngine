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
	VulkanRenderer::RenderStaticData VulkanRenderer::s_RenderData;

	struct UniformBufferObject_TEST
	{
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
	};

	VulkanRenderer::VulkanRenderer()
	{
		s_Instance = this;
	}

	void VulkanRenderer::Initialize(const VkExtent2D& windowSize)
    {
		// initialize renderer
        VulkanAllocator::Initialize();
        s_RenderData.m_Swapchain.Initialize(windowSize);
        s_RenderData.m_CommandPool.Initialize(s_RenderData.m_Specs.FRAMES_IN_FLIGHT);

        s_RenderData.m_Framebuffer = std::make_shared<VulkanImage>(windowSize);

		// setup shaders and pipelines
		s_RenderData.m_Uniforms.resize(s_RenderData.m_Specs.FRAMES_IN_FLIGHT);
		for (auto& uniform : s_RenderData.m_Uniforms)
		{
			uniform = std::make_shared<VulkanBufferUniform>(sizeof(UniformBufferObject_TEST));
		}

		s_RenderData.m_GlobalDescriptorPool = VulkanDescriptorPool::Builder()
			.SetMaxSets(s_RenderData.m_Specs.FRAMES_IN_FLIGHT)
			.AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, s_RenderData.m_Specs.FRAMES_IN_FLIGHT)
			.Build();

		const auto globalSetLayout = VulkanDescriptorSetLayout::Builder()
			.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
			.Build();

		s_RenderData.m_GlobalSets = std::vector<VkDescriptorSet>(s_RenderData.m_Specs.FRAMES_IN_FLIGHT);
		for (int i = 0; i < s_RenderData.m_GlobalSets.size(); ++i)
		{
			const auto& bufferInfo = s_RenderData.m_Uniforms[i]->GetDescriptorInfo();

			DescriptorWriter(*globalSetLayout, *s_RenderData.m_GlobalDescriptorPool)
				.WriteBuffer(0, bufferInfo)
				.Build(s_RenderData.m_GlobalSets[i]);
		}

		const auto fragment = std::make_shared<VulkanShader>(RESOURCES_PATH "triangle.frag.spv", ShaderType::FRAGMENT);
		const auto triangleVertexShader = std::make_shared<VulkanShader>(RESOURCES_PATH "triangle_meshed.vert.spv", ShaderType::VERTEX);

		const auto meshedSpecs = VulkanGraphicsPipelineSpecs
		{
			fragment,
			triangleVertexShader,
			s_RenderData.m_Framebuffer,

			VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			VK_POLYGON_MODE_FILL,
			VK_CULL_MODE_NONE,
			VK_FRONT_FACE_CLOCKWISE,
		};
		s_RenderData.m_MeshedPipeline = VulkanGraphicsPipeline(meshedSpecs, globalSetLayout->GetDescriptorSetLayout());

		s_RenderData.m_ShaderLibrary.Add(fragment);
		fragment->Release();
		s_RenderData.m_ShaderLibrary.Add(triangleVertexShader);
		triangleVertexShader->Release();

		// setup resize function
	    s_RenderData.m_Swapchain.QueueResize([&]
	    {
            const auto& framebufferSize = Application::Get().GetWindow().GetWindowSize();

            // recreate framebuffer
            s_RenderData.m_Framebuffer = std::make_shared<VulkanImage>(framebufferSize);

            // TODO: recreate pipelines
	    });
	}

	void VulkanRenderer::ImmediateSubmit(std::function<void(VkCommandBuffer freeBuffer)>&& function)
	{
		const auto immediateBuffer = s_RenderData.m_CommandPool.GetImmediateBuffer();
		s_RenderData.m_CommandPool.BeginImmediateQueue();

		function(immediateBuffer);

		s_RenderData.m_CommandPool.EndImmediateQueue();
	}

	void VulkanRenderer::PrepareFrame()
	{
		s_RenderData.m_BufferData = s_RenderData.m_CommandPool.GetNextFrame();

		// begin render queue
		s_RenderData.m_CommandPool.BeginCommandQueue();

		// prepare swapchain
		s_RenderData.m_Swapchain.UpdateFrame(s_RenderData.m_BufferData);
		const auto& swapchainImage = s_RenderData.m_Swapchain.GetCurrentFrame();

		// render commands
		// transit framebuffer into writeable mod
		VulkanImage::Transit(s_RenderData.m_Framebuffer->GetRawImage(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	}

	void VulkanRenderer::EndFrame()
	{
		const auto& buffer = s_RenderData.m_BufferData.CommandBuffer;
		const auto& swapchainImage = s_RenderData.m_Swapchain.GetCurrentFrame();

		// copy framebuffer into swapchain
		VulkanImage::Transit(s_RenderData.m_Framebuffer->GetRawImage(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		VulkanImage::Transit(swapchainImage.ImageData, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		s_RenderData.m_Framebuffer->CopyRaw(swapchainImage.ImageData, s_RenderData.m_Swapchain.GetSwapchainSize());
		VulkanImage::Transit(swapchainImage.ImageData, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

		// setup render infos
		auto colorAttachment = VkRenderingAttachmentInfo();
		colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		colorAttachment.imageView = swapchainImage.ImageViewData;
		colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

		auto renderInfo = VkRenderingInfo();
		renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
		renderInfo.renderArea = VkRect2D(VkOffset2D(0, 0), s_RenderData.m_Swapchain.GetSwapchainSize());
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
		s_RenderData.m_CommandPool.EndCommandQueue();

		// render swapchain image
		s_RenderData.m_Swapchain.ShowSwapchain(s_RenderData.m_BufferData);
		s_RenderData.m_CurrentFrameIndex = (s_RenderData.m_CurrentFrameIndex + 1) % s_RenderData.m_Specs.FRAMES_IN_FLIGHT;
	}

	void VulkanRenderer::RenderVertexArray(const std::shared_ptr<VulkanVertexArray>& vertexArray)
	{
		const auto& buffer = s_RenderData.m_BufferData.CommandBuffer;

		// setup render infos
		auto colorAttachment = VkRenderingAttachmentInfo();
		colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		colorAttachment.imageView = s_RenderData.m_Framebuffer->GetImageView();
		colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

		auto renderInfo = VkRenderingInfo();
		renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
		renderInfo.renderArea = VkRect2D(VkOffset2D(0, 0), s_RenderData.m_Swapchain.GetSwapchainSize());
		renderInfo.layerCount = 1;
		renderInfo.colorAttachmentCount = 1;
		renderInfo.pColorAttachments = &colorAttachment;

		// draw geometry
		vkCmdBeginRendering(buffer, &renderInfo);

		//set dynamic viewport and scissor
		const auto& [width, height] = s_RenderData.m_Swapchain.GetSwapchainSize();

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

		s_RenderData.m_Uniforms[s_RenderData.m_CurrentFrameIndex]->WriteData(&ubo, sizeof(ubo));

		//launch a draw command to draw vertices
		vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, s_RenderData.m_MeshedPipeline.GetPipeline());

		VkBuffer vertexBuffers[] = { vertexArray->GetVertexBuffer().Buffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(buffer, 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(buffer, vertexArray->GetIndexBuffer().Buffer, 0, VK_INDEX_TYPE_UINT32);

		auto set = s_RenderData.m_GlobalSets[s_RenderData.m_CurrentFrameIndex];
		vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, s_RenderData.m_MeshedPipeline.GetPipelineLayout(), 0, 1, &set, 0, nullptr);
		vkCmdDrawIndexed(buffer, vertexArray->GetIndexBufferSize(), 1, 0, 0, 0);

		vkCmdEndRendering(buffer);
	}

	void VulkanRenderer::Shutdown() 
    {
		s_RenderData.m_CommandPool.Destroy();
		s_RenderData.m_Swapchain.Destroy();

		s_RenderData.m_MeshedPipeline.Destroy();
		s_RenderData.m_ShaderLibrary.Destroy();

		// clear resources
		s_RenderData.m_Framebuffer = nullptr;
		s_RenderData.m_GlobalDescriptorPool = nullptr;
		s_RenderData.m_GlobalSets.clear();
		s_RenderData.m_Uniforms.clear();
	}
}
