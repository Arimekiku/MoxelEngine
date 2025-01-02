#include "scene_layer.h"
#include "engine/vulkan/vulkan_renderer.h"

#include <imgui.h>

namespace SDLarria
{
	SceneLayer::SceneLayer()
	{
		auto vertices = std::vector<Vertex>(8);
		vertices[0].Position = glm::vec3(-0.5f, -0.5f, 0.5f);
		vertices[1].Position = glm::vec3( 0.5f, -0.5f, 0.5f);
		vertices[2].Position = glm::vec3( 0.5f,  0.5f, 0.5f);
		vertices[3].Position = glm::vec3(-0.5f,  0.5f, 0.5f);
		vertices[4].Position = glm::vec3(-0.5f, -0.5f, -0.5f);
		vertices[5].Position = glm::vec3( 0.5f, -0.5f, -0.5f);
		vertices[6].Position = glm::vec3( 0.5f,  0.5f, -0.5f);
		vertices[7].Position = glm::vec3(-0.5f,  0.5f, -0.5f);

		vertices[0].Color = glm::vec3(1, 1, 1);
		vertices[1].Color = glm::vec3(1, 1, 1);
		vertices[2].Color = glm::vec3(1, 1, 1);
		vertices[3].Color = glm::vec3(1, 1, 1);
		vertices[4].Color = glm::vec3(1, 1, 1);
		vertices[5].Color = glm::vec3(1, 1, 1);
		vertices[6].Color = glm::vec3(1, 1, 1);
		vertices[7].Color = glm::vec3(1, 1, 1);

		/*vertices[0].Normal = glm::vec3(-1.0f, -1.0f,  1.0f);
		vertices[1].Normal = glm::vec3( 1.0f, -1.0f,  1.0f);
		vertices[2].Normal = glm::vec3( 1.0f,  1.0f,  1.0f);
		vertices[3].Normal = glm::vec3(-1.0f,  1.0f,  1.0f);
		vertices[4].Normal = glm::vec3(-1.0f, -1.0f, -1.0f);
		vertices[5].Normal = glm::vec3( 1.0f, -1.0f, -1.0f);
		vertices[6].Normal = glm::vec3( 1.0f,  1.0f, -1.0f);
		vertices[7].Normal = glm::vec3(-1.0f,  1.0f, -1.0f);*/

		std::vector<uint32_t> indices =
		{
			0, 1, 2, 2, 3, 0,
			1, 5, 6, 6, 2, 1,
			7, 6, 5, 5, 4, 7,
			4, 0, 3, 3, 7, 4,
			4, 5, 1, 1, 0, 4,
			3, 2, 6, 6, 7, 3,
		};

		m_Cube = std::make_shared<VulkanVertexArray>(indices, vertices);

		m_renderCam = RenderCamera(glm::vec3(0, 0, -1), glm::vec3(0, 0, 1));
	}

	void SceneLayer::OnEveryUpdate()
	{
		m_renderCam.Update();

		VulkanRenderer::RenderVertexArray(m_renderCam.GetProjViewMat(), m_Cube);
	}

	void SceneLayer::OnGuiUpdate()
	{
		ImGui::ShowDemoWindow();
	}

	void SceneLayer::Detach()
	{
		m_Cube = nullptr;
	}
}
