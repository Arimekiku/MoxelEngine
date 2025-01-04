#include "vulkan_renderer.h"
#include "renderer/application.h"
#include "vulkan.h"

#include <backends/imgui_impl_vulkan.h>

namespace Moxel
{
	VulkanRenderer::RenderStaticData VulkanRenderer::s_RenderData;

	struct GlobalRenderData
	{
		glm::mat4 meshTRS;
		glm::mat4 cameraMatrix;
	};

	void VulkanRenderer::Initialize(const VkExtent2D& windowSize)
	{
		// initialize renderer
		s_RenderData.m_Swapchain.Initialize(windowSize);
		s_RenderData.m_CommandPool.Initialize(s_RenderData.m_Specs.FRAMES_IN_FLIGHT);

		// setup shaders and pipelines
		s_RenderData.m_Uniforms.resize(s_RenderData.m_Specs.FRAMES_IN_FLIGHT);
		for (auto& uniform : s_RenderData.m_Uniforms)
		{
			uniform = std::make_shared<VulkanBufferUniform>(sizeof(GlobalRenderData));
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
			s_RenderData.m_Swapchain.GetFramebuffer()->GetRenderImage(),

			VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			VK_POLYGON_MODE_FILL,
			VK_CULL_MODE_FRONT_BIT,
			VK_FRONT_FACE_CLOCKWISE,
		};
		s_RenderData.m_MeshedPipeline = VulkanGraphicsPipeline(meshedSpecs, globalSetLayout->GetDescriptorSetLayout());

		// release shaders
		s_RenderData.m_ShaderLibrary.Add(fragment);
		fragment->Release();
		s_RenderData.m_ShaderLibrary.Add(triangleVertexShader);
		triangleVertexShader->Release();
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
		const auto& buffer = s_RenderData.m_BufferData.CommandBuffer;
		const auto& framebuffer = s_RenderData.m_Swapchain.GetFramebuffer();

		// begin render queue
		s_RenderData.m_CommandPool.BeginCommandQueue();

		// prepare swapchain
		s_RenderData.m_Swapchain.UpdateFrame(s_RenderData.m_BufferData);

		// clear framebuffer
		VulkanImage::Transit(framebuffer->GetRenderImage()->GetRawImage(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

		auto vulkanSubresourceRange = VkImageSubresourceRange();
		vulkanSubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		vulkanSubresourceRange.levelCount = 1;
		vulkanSubresourceRange.layerCount = 1;

		constexpr auto clearColor = VkClearColorValue
		{
			164.0f / 256.0f,
			30.0f / 256.0f,
			34.0f / 256.0f,
			0.0f
		};
		auto clearValue = VkClearValue();
		clearValue.color = clearColor;

		vkCmdClearColorImage(buffer, 
			framebuffer->GetRenderImage()->GetRawImage(),
			VK_IMAGE_LAYOUT_GENERAL,
			reinterpret_cast<VkClearColorValue*>(&clearValue),
			1,
			&vulkanSubresourceRange);

		framebuffer->Bind();
	}

	void VulkanRenderer::EndFrame()
	{
		const auto& buffer = s_RenderData.m_BufferData.CommandBuffer;
		const auto& swapchainImage = s_RenderData.m_Swapchain.GetCurrentFrame();
		const auto& framebuffer = s_RenderData.m_Swapchain.GetFramebuffer();

		// copy framebuffer into swapchain
		VulkanImage::Transit(framebuffer->GetRenderImage()->GetRawImage(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
							 VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		VulkanImage::Transit(swapchainImage.ImageData, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		framebuffer->GetRenderImage()->CopyRaw(swapchainImage.ImageData, s_RenderData.m_Swapchain.GetSwapchainSize());
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

	void VulkanRenderer::RenderVertexArray(const std::shared_ptr<RenderMesh>& mesh, const glm::mat4& cameraMat)
	{
		const auto& buffer = s_RenderData.m_BufferData.CommandBuffer;
		const auto& framebuffer = s_RenderData.m_Swapchain.GetFramebuffer();

		auto renderInfo = VkRenderingInfo();
		renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
		renderInfo.renderArea = VkRect2D(VkOffset2D(0, 0), s_RenderData.m_Swapchain.GetSwapchainSize());
		renderInfo.layerCount = 1;
		renderInfo.colorAttachmentCount = 1;
		renderInfo.pColorAttachments = &framebuffer->GetColorAttachment();
		renderInfo.pDepthAttachment = &framebuffer->GetDepthAttachment();

		// draw geometry
		vkCmdBeginRendering(buffer, &renderInfo);

		//set dynamic viewport and scissor
		const auto& [width, height] = s_RenderData.m_Swapchain.GetSwapchainSize();

		auto viewport = VkViewport();
		viewport.x = 0;
		viewport.y = 0;
		viewport.width = static_cast<float>(width);
		viewport.height = static_cast<float>(height);
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
		auto ubo = GlobalRenderData();
		ubo.meshTRS = mesh->GetTRSMatrix();
		ubo.cameraMatrix = cameraMat;
		s_RenderData.m_Uniforms[s_RenderData.m_CurrentFrameIndex]->WriteData(&ubo, sizeof(ubo));

		//launch a draw command to draw vertices
		vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, s_RenderData.m_MeshedPipeline.GetPipeline());

		const auto& vertexArray = mesh->GetVertexArray();
		VkBuffer vertexBuffer = vertexArray->GetVertexBuffer().Buffer;
		constexpr VkDeviceSize offsets[] = { 0 };

		vkCmdBindVertexBuffers(buffer, 0, 1, &vertexBuffer, offsets);
		vkCmdBindIndexBuffer(buffer, vertexArray->GetIndexBuffer().Buffer, 0, VK_INDEX_TYPE_UINT32);

		const auto& set = s_RenderData.m_GlobalSets[s_RenderData.m_CurrentFrameIndex];
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
		s_RenderData.m_GlobalDescriptorPool = nullptr;
		s_RenderData.m_GlobalSets.clear();
		s_RenderData.m_Uniforms.clear();
	}
}
