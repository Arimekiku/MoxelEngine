#include "vulkan_renderer.h"
#include "engine/application.h"
#include "vulkan.h"
#include "vulkan_allocator.h"

#include <backends/imgui_impl_vulkan.h>

namespace Moxel
{
	VulkanRenderer::RenderStaticData VulkanRenderer::s_renderData;
	std::vector<std::function<void()>> VulkanRenderer::s_deletionQueue;

	struct GlobalRenderData
	{
		glm::mat4 CameraMatrix;
	};

	void VulkanRenderer::initialize(const VkExtent2D& windowSize)
	{
		// initialize renderer
		s_renderData.Swapchain.initialize(windowSize);
		s_renderData.CommandPool.initialize(s_renderData.Specs.FRAMES_IN_FLIGHT);

		// setup shaders and pipelines
		s_renderData.Uniforms.resize(s_renderData.Specs.FRAMES_IN_FLIGHT);
		for (auto& uniform : s_renderData.Uniforms)
		{
			uniform = std::make_shared<VulkanBufferUniform>(sizeof(GlobalRenderData));
		}

		s_renderData.GlobalDescriptorPool = VulkanDescriptorPool::Builder()
			.with_max_sets(s_renderData.Specs.FRAMES_IN_FLIGHT)
			.add_pool_size(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, s_renderData.Specs.FRAMES_IN_FLIGHT)
			.build();

		const auto globalSetLayout = VulkanDescriptorSetLayout::Builder()
			.add_binding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
			.build();

		s_renderData.GlobalSets = std::vector<VkDescriptorSet>(s_renderData.Specs.FRAMES_IN_FLIGHT);
		for (int i = 0; i < s_renderData.GlobalSets.size(); ++i)
		{
			const auto& bufferInfo = s_renderData.Uniforms[i]->get_descriptor_info();

			DescriptorWriter(*globalSetLayout, *s_renderData.GlobalDescriptorPool)
				.write_buffer(0, bufferInfo)
				.build(s_renderData.GlobalSets[i]);
		}

		const auto fragment = std::make_shared<VulkanShader>(RESOURCES_PATH "triangle.frag.spv", ShaderType::FRAGMENT);
		const auto vertex = std::make_shared<VulkanShader>(RESOURCES_PATH "triangle_meshed.vert.spv", ShaderType::VERTEX);

		auto pushConstant = VkPushConstantRange();
		pushConstant.offset = 0;
		pushConstant.size = sizeof(glm::mat4);
		pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		s_renderData.MeshedPipeline = VulkanGraphicsPipeline::Builder()
			.with_fragment(fragment->get_pipeline_create_info())
			.with_vertex(vertex->get_pipeline_create_info())
		// TODO: this is a MESS, should think about it
			.add_color_attachment(s_renderData.Swapchain.get_framebuffer()->get_render_image()->get_image_asset().ImageFormat)
			.with_depth_attachment(s_renderData.Swapchain.get_framebuffer()->get_depth_image()->get_image_asset().ImageFormat)
		//
			.add_push_constant(pushConstant)
			.add_layout(globalSetLayout->get_descriptor_set_layout())
			.build();

		// release shaders
		s_renderData.ShaderLibrary.add(fragment);
		fragment->release();
		s_renderData.ShaderLibrary.add(vertex);
		vertex->release();
	}

	void VulkanRenderer::immediate_submit(std::function<void(VkCommandBuffer freeBuffer)>&& function)
	{
		const auto immediateBuffer = s_renderData.CommandPool.get_immediate_buffer();
		s_renderData.CommandPool.begin_immediate_queue();

		function(immediateBuffer);

		s_renderData.CommandPool.end_immediate_queue();
	}

	void VulkanRenderer::prepare_frame()
	{
		s_renderData.BufferData = s_renderData.CommandPool.get_next_frame();
		const auto& buffer = s_renderData.BufferData.CommandBuffer;
		const auto& framebuffer = s_renderData.Swapchain.get_framebuffer();

		// begin render queue
		// update fences
		const auto device = Application::get().get_context().get_logical_device();

		auto result = vkWaitForFences(device, 1, &s_renderData.BufferData.RenderFence, true, 1000000000);
		VULKAN_CHECK(result);

		for (const auto& function: s_deletionQueue)
		{
			function();
		}
		s_deletionQueue.clear();

		result = vkResetFences(device, 1, &s_renderData.BufferData.RenderFence);
		VULKAN_CHECK(result);

		s_renderData.CommandPool.begin_command_queue();

		// prepare swapchain
		s_renderData.Swapchain.update_frame(s_renderData.BufferData);

		// clear color image with pink color
		framebuffer->get_render_image()->clear({
				164.0f / 256.0f,
				30.0f / 256.0f,
				34.0f / 256.0f,
		});

		// bind dynamic resources
		framebuffer->bind();

		auto renderInfo = VkRenderingInfo();
		renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
		renderInfo.renderArea = VkRect2D(VkOffset2D(0, 0), s_renderData.Swapchain.get_swapchain_size());
		renderInfo.layerCount = 1;
		renderInfo.colorAttachmentCount = 1;
		renderInfo.pColorAttachments = &framebuffer->get_color_attachment();
		renderInfo.pDepthAttachment = &framebuffer->get_depth_attachment();

		// begin framebuffer rendering
		vkCmdBeginRendering(buffer, &renderInfo);

		// set dynamic viewport and scissor
		const auto& [width, height] = s_renderData.Swapchain.get_swapchain_size();

		auto viewport = VkViewport();
		viewport.x = 0;
		viewport.y = 0;
		viewport.width = static_cast<float>(width);
		viewport.height = static_cast<float>(height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(buffer, 0, 1, &viewport);

		auto scissor = VkRect2D();
		scissor.offset.x = 0;
		scissor.offset.y = 0;
		scissor.extent.width = width;
		scissor.extent.height = height;
		vkCmdSetScissor(buffer, 0, 1, &scissor);
	}

	void VulkanRenderer::end_frame()
	{
		const auto& buffer = s_renderData.BufferData.CommandBuffer;
		const auto& swapchainImage = s_renderData.Swapchain.get_current_frame();
		const auto& framebuffer = s_renderData.Swapchain.get_framebuffer();

		// end framebuffer rendering
		vkCmdEndRendering(buffer);

		// copy framebuffer into swapchain
		VulkanImage::transit(framebuffer->get_render_image()->get_image_asset().Image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		VulkanImage::transit(swapchainImage.ImageData, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		framebuffer->get_render_image()->copy_raw(swapchainImage.ImageData, s_renderData.Swapchain.get_swapchain_size());
		VulkanImage::transit(swapchainImage.ImageData, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

		// setup render infos
		auto colorAttachment = VkRenderingAttachmentInfo();
		colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		colorAttachment.imageView = swapchainImage.ImageViewData;
		colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

		auto renderInfo = VkRenderingInfo();
		renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
		renderInfo.renderArea = VkRect2D(VkOffset2D(0, 0), s_renderData.Swapchain.get_swapchain_size());
		renderInfo.layerCount = 1;
		renderInfo.colorAttachmentCount = 1;
		renderInfo.pColorAttachments = &colorAttachment;

		// draw imgui
		vkCmdBeginRendering(buffer, &renderInfo);

		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), buffer);

		vkCmdEndRendering(buffer);

		// set current mode into present so we can draw it on screen
		VulkanImage::transit(swapchainImage.ImageData, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

		// end render queue
		s_renderData.CommandPool.end_command_queue();

		// render swapchain image
		s_renderData.Swapchain.show_swapchain(s_renderData.BufferData);
		s_renderData.CurrentFrameIndex = (s_renderData.CurrentFrameIndex + 1) % s_renderData.Specs.FRAMES_IN_FLIGHT;
	}

	void VulkanRenderer::render_chunk(const ChunkPosition chunkPosition, const std::shared_ptr<ChunkMesh>& chunk, const glm::mat4& cameraMat)
	{
		const auto& buffer = s_renderData.BufferData.CommandBuffer;

		// update uniform data
		auto ubo = GlobalRenderData();
		ubo.CameraMatrix = cameraMat;
		s_renderData.Uniforms[s_renderData.CurrentFrameIndex]->write_data(&ubo, sizeof(ubo));

		// launch a draw command to draw vertices
		vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, s_renderData.MeshedPipeline->get_pipeline());

		// update per-vertex data
		const glm::vec3 chunkWorld = glm::vec3(chunkPosition.X, chunkPosition.Y, chunkPosition.Z) * 16.0f; 
		vkCmdPushConstants(buffer, s_renderData.MeshedPipeline->get_pipeline_layout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::vec3), &chunkWorld);

		const auto& vertexArray = chunk->get_chunk_mesh();
		VkBuffer vertexBuffer = vertexArray->get_vertex_buffer().Buffer;
		constexpr VkDeviceSize offsets[] = { 0 };

		vkCmdBindVertexBuffers(buffer, 0, 1, &vertexBuffer, offsets);
		vkCmdBindIndexBuffer(buffer, vertexArray->get_index_buffer().Buffer, 0, VK_INDEX_TYPE_UINT32);

		const auto& set = s_renderData.GlobalSets[s_renderData.CurrentFrameIndex];
		vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, s_renderData.MeshedPipeline->get_pipeline_layout(), 0, 1, &set, 0, nullptr);
		vkCmdDrawIndexed(buffer, vertexArray->get_index_buffer_size(), 1, 0, 0, 0);
	}

	void VulkanRenderer::shutdown()
	{
		s_renderData.CommandPool.destroy();
		s_renderData.Swapchain.destroy();

		s_renderData.ShaderLibrary.destroy();
		s_renderData.MeshedPipeline = nullptr;

		s_renderData.GlobalDescriptorPool = nullptr;
		s_renderData.GlobalSets.clear();
		s_renderData.Uniforms.clear();

		for (const auto& function: s_deletionQueue)
		{
			function();
		}
		s_deletionQueue.clear();
	}
}
