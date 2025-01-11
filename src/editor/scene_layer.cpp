#include "scene_layer.h"
#include "renderer/vulkan/vulkan_renderer.h"

#include <imgui.h>

namespace Moxel
{
	SceneLayer::SceneLayer()
	{
		constexpr auto cameraPosition = glm::vec3(0, 0, 1);
		m_camera = RenderCamera(cameraPosition, glm::vec3(0, 0, -1));
	}

	void SceneLayer::OnEveryUpdate()
	{
		m_camera.Update();

		const auto cameraPosition = m_camera.GetPosition();
		m_chunks.Update(cameraPosition);

		// render chunks
		auto& renderChunks = m_chunks.GetRenderChunks();
		for (const auto& chunk: renderChunks)
		{
			if (chunk->IsSetForDeletion() || chunk->IsEmpty())
			{
				continue;
			}

			VulkanRenderer::RenderVertexArray(chunk->GetVertexArray(), m_camera.GetProjViewMat());
		}
		renderChunks.clear();
	}

	void SceneLayer::OnGuiUpdate()
	{
		ImGui::ShowDemoWindow();
	}

	void SceneLayer::Detach()
	{ 
		m_chunks.DestroyWorld();
	}
}
