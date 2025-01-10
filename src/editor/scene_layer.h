#pragma once

#include "renderer/core/layer/layer.h"
#include "engine/render_camera.h"
#include "engine/voxel_chunk.h"

struct KeyHasher
{
	size_t operator()(const glm::vec3& a) const { return std::hash<float>{}(a.x + a.y + a.z); }
};

struct KeyEquals
{
	bool operator()(const glm::vec3& a, const glm::vec3& b) const 
	{ 
		return a.x == b.x && a.y == b.y && a.z == b.z; 
	}
};

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
		int m_RenderDistance = 5;
		RenderCamera m_RenderCam;

		std::unordered_map<glm::vec3, VoxelChunk, KeyHasher, KeyEquals> m_TotalChunks;
		std::vector<VoxelChunk> m_RenderChunks;
	};
}
