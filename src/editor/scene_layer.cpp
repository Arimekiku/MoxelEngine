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

	void SceneLayer::on_every_update()
	{
		m_camera.update();

		const auto cameraPosition = m_camera.get_position();
		m_chunks.update(cameraPosition);

		// render chunks
		auto& renderChunks = m_chunks.get_render_queue();
		while (renderChunks.empty() == false)
		{
			const auto& position = renderChunks.front().first;
			const auto& chunk = renderChunks.front().second;

			VulkanRenderer::render_chunk(position, chunk, m_camera.get_proj_view_mat());
			m_verticesCount += chunk->get_chunk_mesh()->get_vertices().size();

			renderChunks.pop();
		}
	}

	void SceneLayer::on_gui_update()
	{
		ImGui::ShowDemoWindow();

		ImGui::Begin("Stats");

		ImGui::Text("Chunks Generated: %d", m_chunks.get_total_chunks_data_count());
		ImGui::Text("Meshes Generated: %d", m_chunks.get_total_chunks_mesh_count());
		ImGui::Text("Vertices Rendered: %d", m_verticesCount);

		ImGui::End();

		m_verticesCount = 0;
	}

	void SceneLayer::detach()
	{ 
		m_chunks.destroy_world();
	}
}
