#include "vulkan_renderer.h"
#include "renderer/application.h"
#include "vulkan.h"
#include "vulkan_allocator.h"

#include <backends/imgui_impl_vulkan.h>

namespace Moxel
{
	VulkanRenderer::RenderStaticData VulkanRenderer::s_renderData;
	std::vector<std::shared_ptr<VulkanVertexArray>> VulkanRenderer::s_deletionQueue;

	struct GlobalRenderData
	{
		glm::mat4 cameraMatrix;
	};

	void VulkanRenderer::initialize(const VkExtent2D& windowSize)
	{
		// initialize renderer
		s_renderData.m_swapchain.initialize(windowSize);
		s_renderData.m_commandPool.initialize(s_renderData.m_specs.FRAMES_IN_FLIGHT);

		// setup shaders and pipelines
		s_renderData.m_uniforms.resize(s_renderData.m_specs.FRAMES_IN_FLIGHT);
		for (auto& uniform : s_renderData.m_uniforms)
		{
			uniform = std::make_shared<VulkanBufferUniform>(sizeof(GlobalRenderData));
		}

		s_renderData.m_globalDescriptorPool = VulkanDescriptorPool::Builder()
			.set_max_sets(s_renderData.m_specs.FRAMES_IN_FLIGHT)
			.add_pool_size(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, s_renderData.m_specs.FRAMES_IN_FLIGHT)
			.build();

		const auto globalSetLayout = VulkanDescriptorSetLayout::Builder()
			.add_binding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
			.build();

		s_renderData.m_globalSets = std::vector<VkDescriptorSet>(s_renderData.m_specs.FRAMES_IN_FLIGHT);
		for (int i = 0; i < s_renderData.m_globalSets.size(); ++i)
		{
			const auto& bufferInfo = s_renderData.m_uniforms[i]->get_descriptor_info();

			DescriptorWriter(*globalSetLayout, *s_renderData.m_globalDescriptorPool)
				.write_buffer(0, bufferInfo)
				.build(s_renderData.m_globalSets[i]);
		}

		const auto fragment = std::make_shared<VulkanShader>(RESOURCES_PATH "triangle.frag.spv", ShaderType::FRAGMENT);
		const auto triangleVertexShader = std::make_shared<VulkanShader>(RESOURCES_PATH "triangle_meshed.vert.spv", ShaderType::VERTEX);

		auto pushConstant = VkPushConstantRange();
		pushConstant.offset = 0;
		pushConstant.size = sizeof(glm::mat4);
		pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		const auto meshedSpecs = VulkanGraphicsPipelineSpecs
		{
			fragment,
			triangleVertexShader,
			s_renderData.m_swapchain.get_framebuffer()->get_render_image(),

			VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			VK_POLYGON_MODE_FILL,
			VK_CULL_MODE_FRONT_BIT,
			VK_FRONT_FACE_CLOCKWISE,
			pushConstant
		};
		s_renderData.m_meshedPipeline = VulkanGraphicsPipeline(meshedSpecs, globalSetLayout->get_descriptor_set_layout());

		// release shaders
		s_renderData.m_shaderLibrary.add(fragment);
		fragment->release();
		s_renderData.m_shaderLibrary.add(triangleVertexShader);
		triangleVertexShader->release();
	}

	void VulkanRenderer::immediate_submit(std::function<void(VkCommandBuffer freeBuffer)>&& function)
	{
		const auto immediateBuffer = s_renderData.m_commandPool.get_immediate_buffer();
		s_renderData.m_commandPool.begin_immediate_queue();

		function(immediateBuffer);

		s_renderData.m_commandPool.end_immediate_queue();
	}

	void VulkanRenderer::prepare_frame()
	{
		s_renderData.m_bufferData = s_renderData.m_commandPool.get_next_frame();
		const auto& buffer = s_renderData.m_bufferData.CommandBuffer;
		const auto& framebuffer = s_renderData.m_swapchain.get_framebuffer();

		// begin render queue
		// update fences
		const auto device = Application::get().get_context().get_logical_device();

		auto result = vkWaitForFences(device, 1, &s_renderData.m_bufferData.RenderFence, true, 1000000000);
		VULKAN_CHECK(result);

		auto& allocator = Application::get().get_allocator();
		for (const auto& vertexArray: s_deletionQueue)
		{
			allocator.destroy_buffer(vertexArray->get_vertex_buffer());
			allocator.destroy_buffer(vertexArray->get_index_buffer());
		}
		s_deletionQueue.clear();

		result = vkResetFences(device, 1, &s_renderData.m_bufferData.RenderFence);
		VULKAN_CHECK(result);

		s_renderData.m_commandPool.begin_command_queue();

		// prepare swapchain
		s_renderData.m_swapchain.update_frame(s_renderData.m_bufferData);

		// clear resources
		VulkanImage::transit(framebuffer->get_render_image()->get_image_asset().Image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

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
			framebuffer->get_render_image()->get_image_asset().Image,
			VK_IMAGE_LAYOUT_GENERAL,
			reinterpret_cast<VkClearColorValue*>(&clearValue),
			1,
			&vulkanSubresourceRange);

		// bind dynamic resources
		framebuffer->bind();

		auto renderInfo = VkRenderingInfo();
		renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
		renderInfo.renderArea = VkRect2D(VkOffset2D(0, 0), s_renderData.m_swapchain.get_swapchain_size());
		renderInfo.layerCount = 1;
		renderInfo.colorAttachmentCount = 1;
		renderInfo.pColorAttachments = &framebuffer->get_color_attachment();
		renderInfo.pDepthAttachment = &framebuffer->get_depth_attachment();

		// begin framebuffer rendering
		vkCmdBeginRendering(buffer, &renderInfo);

		// set dynamic viewport and scissor
		const auto& [width, height] = s_renderData.m_swapchain.get_swapchain_size();

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
		const auto& buffer = s_renderData.m_bufferData.CommandBuffer;
		const auto& swapchainImage = s_renderData.m_swapchain.get_current_frame();
		const auto& framebuffer = s_renderData.m_swapchain.get_framebuffer();

		// end framebuffer rendering
		vkCmdEndRendering(buffer);

		// copy framebuffer into swapchain
		VulkanImage::transit(framebuffer->get_render_image()->get_image_asset().Image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		VulkanImage::transit(swapchainImage.ImageData, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		framebuffer->get_render_image()->copy_raw(swapchainImage.ImageData, s_renderData.m_swapchain.get_swapchain_size());
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
		renderInfo.renderArea = VkRect2D(VkOffset2D(0, 0), s_renderData.m_swapchain.get_swapchain_size());
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
		s_renderData.m_commandPool.end_command_queue();

		// render swapchain image
		s_renderData.m_swapchain.show_swapchain(s_renderData.m_bufferData);
		s_renderData.m_currentFrameIndex = (s_renderData.m_currentFrameIndex + 1) % s_renderData.m_specs.FRAMES_IN_FLIGHT;
	}

	void VulkanRenderer::render_chunk(const ChunkPosition chunkPosition, const std::shared_ptr<ChunkMesh>& chunk, const glm::mat4& cameraMat)
	{
		const auto& buffer = s_renderData.m_bufferData.CommandBuffer;

		// update uniform data
		auto ubo = GlobalRenderData();
		ubo.cameraMatrix = cameraMat;
		s_renderData.m_uniforms[s_renderData.m_currentFrameIndex]->write_data(&ubo, sizeof(ubo));

		// launch a draw command to draw vertices
		vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, s_renderData.m_meshedPipeline.get_pipeline());

		// update per-vertex data
		const glm::vec3 chunkWorld = glm::vec3(chunkPosition.X, chunkPosition.Y, chunkPosition.Z) * 16.0f; 
		vkCmdPushConstants(buffer, s_renderData.m_meshedPipeline.get_pipeline_layout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::vec3), &chunkWorld);

		const auto& vertexArray = chunk->get_chunk_mesh();
		VkBuffer vertexBuffer = vertexArray->get_vertex_buffer().Buffer;
		constexpr VkDeviceSize offsets[] = { 0 };

		vkCmdBindVertexBuffers(buffer, 0, 1, &vertexBuffer, offsets);
		vkCmdBindIndexBuffer(buffer, vertexArray->get_index_buffer().Buffer, 0, VK_INDEX_TYPE_UINT32);

		const auto& set = s_renderData.m_globalSets[s_renderData.m_currentFrameIndex];
		vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, s_renderData.m_meshedPipeline.get_pipeline_layout(), 0, 1, &set, 0, nullptr);
		vkCmdDrawIndexed(buffer, vertexArray->get_index_buffer_size(), 1, 0, 0, 0);
	}

	void VulkanRenderer::shutdown()
	{
		s_renderData.m_commandPool.destroy();
		s_renderData.m_swapchain.destroy();

		s_renderData.m_meshedPipeline.destroy();
		s_renderData.m_shaderLibrary.destroy();

		// clear resources
		s_renderData.m_globalDescriptorPool = nullptr;
		s_renderData.m_globalSets.clear();
		s_renderData.m_uniforms.clear();

		auto& allocator = Application::get().get_allocator();
		for (const auto& vertexArray: s_deletionQueue)
		{
			allocator.destroy_buffer(vertexArray->get_vertex_buffer());
			allocator.destroy_buffer(vertexArray->get_index_buffer());
		}
		s_deletionQueue.clear();
	}
}
