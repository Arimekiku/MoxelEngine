#pragma once

#include "renderer/core/layer/layer.h"
#include "engine/render_camera.h"
#include "engine/chunk_generator.h"
#include "renderer/vulkan/vulkan_image.h"

namespace Moxel
{
	class SceneLayer final : public Layer
	{
	public:
		SceneLayer();

		void detach() override;

		void on_every_update() override;
		void on_gui_update() override;

	private:
		std::shared_ptr<VulkanImage> m_image = nullptr;
		RenderCamera m_camera;
		ChunkBuilder m_chunks;

		int m_verticesCount = 0;
	};
}
