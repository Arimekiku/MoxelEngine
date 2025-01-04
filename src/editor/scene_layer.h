#pragma once

#include "renderer/core/layer/layer.h"
#include "engine/render_mesh.h"
#include "engine/render_camera.h"

#include <memory>

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
		RenderCamera m_RenderCam;

		std::shared_ptr<RenderMesh> m_Cube;
	};
}
