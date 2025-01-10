#include "scene_layer.h"
#include "renderer/vulkan/vulkan_renderer.h"

#include <imgui.h>
#include <algorithm>
#include <queue>

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

					m_TotalChunks.emplace(chunkPosition, std::make_shared<VoxelChunk>(123456u, chunkPosition));
					m_TotalChunks[chunkPosition]->RecreateChunk();
				}
			}
		}
	}

	void SceneLayer::OnEveryUpdate()
	{
		m_RenderCam.Update();

		const auto cameraPosition = m_RenderCam.GetPosition();
		const auto intCast = glm::vec3(static_cast<int>(cameraPosition.x / 10.0f), static_cast<int>(cameraPosition.y / 10.0f), static_cast<int>(cameraPosition.z / 10.0f));

		// chunk deletion
		for (const auto& [position, chunk] : m_TotalChunks)
		{
			// TODO: fix IsEmpty() situation
			if (chunk->IsSetForDeletion())
			{
				continue;
			}

			const auto distance = glm::length(chunk->GetPosition() * 10.0f - m_RenderCam.GetPosition());

			if (distance > 100.0f)
			{
				m_UnloadQueue.push(chunk);
				chunk->SetForDeletion();
			}
		}

		for (int i = 0; i < MAX_CHUNKS_PER_FRAME_UNLOADED; i++)
		{
			if (m_UnloadQueue.empty() == true)
			{
				break;
			}

			auto& chunk = m_UnloadQueue.front();

			if (chunk->IsEmpty())
			{
				m_TotalChunks.erase(chunk->GetPosition());
				m_UnloadQueue.pop();
				continue;
			}

			chunk->GetVertexArray()->GetVertexArray()->Destroy();
			m_UnloadQueue.pop();
		}

		// update chunk data
		for (int z = -m_RenderDistance; z < m_RenderDistance; ++z)
		{
			for (int y = -m_RenderDistance; y < m_RenderDistance; ++y)
			{
				for (int x = -m_RenderDistance; x < m_RenderDistance; ++x)
				{
					glm::vec3 chunkPosition = glm::vec3(x, y, z) + intCast;

					if (m_TotalChunks.contains(chunkPosition) == false)
					{
						m_TotalChunks.emplace(chunkPosition, std::make_shared<VoxelChunk>(123456u, chunkPosition));
						m_RenderQueue.push(m_TotalChunks[chunkPosition]);
						continue;
					}

					m_RenderChunks.push_back(m_TotalChunks[chunkPosition]);
				}
			}
		}

		// generate chunks
		std::vector<std::shared_ptr<VoxelChunk>> chunksToParse;
		for (int i = 0; i < MAX_CHUNKS_PER_FRAME_GENERATED; i++)
		{
			if (m_RenderQueue.empty() == true)
			{
				break;
			}

			chunksToParse.push_back(m_RenderQueue.front());
			m_RenderQueue.pop();
		}

		// use parallel for now
		std::ranges::for_each(chunksToParse,
		[this](const std::shared_ptr<VoxelChunk>& chunk)
		{
			chunk->RecreateChunk();
			m_RenderChunks.push_back(chunk);
		});

		// render chunks
		for (const auto& chunk: m_RenderChunks)
		{
			if (chunk->IsSetForDeletion() || chunk->IsEmpty())
			{
				continue;
			}

			VulkanRenderer::RenderVertexArray(chunk->GetVertexArray(), m_RenderCam.GetProjViewMat());
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
