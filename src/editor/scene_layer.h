#pragma once

#include "engine/core/layer/layer.h"
#include "engine/renderer/render_mesh.h"
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
		RenderCamera m_RenderCam;

		std::shared_ptr<RenderMesh> m_Cube;
	};
}
