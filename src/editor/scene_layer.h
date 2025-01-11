#pragma once

#include "renderer/core/layer/layer.h"
#include "engine/render_camera.h"
#include "engine/voxel_chunk.h"
#include <engine/chunk_builder.h>

namespace Moxel
{
	class SceneLayer final : public Layer
	{
	public:
		SceneLayer();

		void Detach() override;

		void OnEveryUpdate() override;
		void OnGuiUpdate() override;

	private:
		RenderCamera m_camera;
		ChunkBuilder m_chunks;
	};
}
