#include "scene_layer.h"
#include "engine/vulkan/vulkan_renderer.h"

#include <imgui.h>

namespace SDLarria
{
	SceneLayer::SceneLayer()
	{
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

		m_Rectangle = std::make_shared<VulkanVertexArray>(rect_indices, rect_vertices);

		std::vector<Vertex> tri_vertices;
		tri_vertices.resize(3);

		tri_vertices[0].Position = { 0.2, -0.5, 0 };
		tri_vertices[1].Position = { 0.5, 0.5, 0 };
		tri_vertices[2].Position = { -0.5, -0.5, 0 };

		tri_vertices[0].Color = { 0, 0, 0 };
		tri_vertices[1].Color = { 0.5, 0.5, 0.5 };
		tri_vertices[2].Color = { 1, 0, 0 };

		std::vector<uint32_t> tri_indices;
		tri_indices.resize(3);

		tri_indices[0] = 0;
		tri_indices[1] = 1;
		tri_indices[2] = 2;

		m_Triangle = std::make_shared<VulkanVertexArray>(tri_indices, tri_vertices);

		m_renderCam = RenderCamera(glm::vec3(2, 2, 2), glm::vec3(-2, -2, -2));
	}

	void SceneLayer::OnEveryUpdate()
	{
		VulkanRenderer::RenderVertexArray(m_renderCam.GetProjViewMat(), m_Rectangle);
		//VulkanRenderer::RenderVertexArray(m_renderCam.GetProjViewMat(), m_Triangle);
	}

	void SceneLayer::OnGuiUpdate()
	{
		ImGui::ShowDemoWindow();
	}

	void SceneLayer::Detach()
	{
		m_Rectangle = nullptr;
	}
}
