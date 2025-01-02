#pragma once

#include "engine/core/layer/layer.h"
#include "engine/vulkan/vulkan_vertex_array.h"
#include "engine/renderer/render_camera.h"

#include <memory>

namespace SDLarria
{
	class SceneLayer final : public Layer
	{
	public:
		SceneLayer();

		void Detach() override;

		void OnEveryUpdate() override;
		void OnGuiUpdate() override;

	private:
		RenderCamera m_renderCam;

		std::shared_ptr<VulkanVertexArray> m_Rectangle;
		std::shared_ptr<VulkanVertexArray> m_Triangle;
	};
}