#include "scene_layer.h"
#include "renderer/vulkan/vulkan_renderer.h"

#include <imgui.h>

namespace Moxel
{
	SceneLayer::SceneLayer()
	{
		constexpr auto cameraPosition = glm::vec3(0, 0, 1);
		m_RenderCam = RenderCamera(cameraPosition, glm::vec3(0, 0, -1));

		for (int z = -m_RenderDistance; z < m_RenderDistance; ++z)
		{
			for (int y = -m_RenderDistance; y < m_RenderDistance; ++y)
			{
				for (int x = -m_RenderDistance; x < m_RenderDistance; ++x)
				{
					const glm::vec3 chunkPosition = glm::vec3(x, y, z) + cameraPosition;

					m_TotalChunks.emplace(chunkPosition, VoxelChunk(123456u));
					m_TotalChunks[chunkPosition].RecreateChunk(chunkPosition);
				}
			}
		}
	}

	void SceneLayer::OnEveryUpdate()
	{
		m_RenderCam.Update();

		const auto cameraPosition = m_RenderCam.GetPosition();
		const auto intCast = glm::vec3((int) (cameraPosition.x / 10.0f), (int) (cameraPosition.y / 10.0f), (int) (cameraPosition.z / 10.0f));
		for (int z = -m_RenderDistance; z < m_RenderDistance; ++z)
		{
			for (int y = -m_RenderDistance; y < m_RenderDistance; ++y)
			{
				for (int x = -m_RenderDistance; x < m_RenderDistance; ++x)
				{
					glm::vec3 chunkPosition = glm::vec3(x, y, z) + intCast;

					if (m_TotalChunks.contains(chunkPosition) == false)
					{
						m_TotalChunks.emplace(chunkPosition, VoxelChunk(123456u));
						m_TotalChunks[chunkPosition].RecreateChunk(chunkPosition);
					}

					m_RenderChunks.emplace_back(m_TotalChunks[chunkPosition]);
				}
			}
		}

		for (const auto& chunk: m_RenderChunks)
		{
			if (chunk.IsFree())
			{
				continue;
			}

			VulkanRenderer::RenderVertexArray(chunk.GetVertexArray(), m_RenderCam.GetProjViewMat());
		}
		m_RenderChunks.clear();
	}

	void SceneLayer::OnGuiUpdate()
	{
		ImGui::ShowDemoWindow();
	}

	void SceneLayer::Detach()
	{ 
		m_TotalChunks.clear();
	}
}
