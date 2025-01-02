#pragma once

#include "engine/core/layer/layer.h"
#include "engine/vulkan/vulkan_vertex_array.h"

#include <memory>

namespace SDLarria
{
	class SceneLayer : public Layer
	{
	public:
		SceneLayer();

		void Detach() override;

		void OnEveryUpdate() override;
		void OnGuiUpdate() override;

	private:
		std::shared_ptr<VulkanVertexArray> m_Rectangle;
		std::shared_ptr<VulkanVertexArray> m_Triangle;
	};
}