#pragma once

#include "renderer/core/layer/layer.h"
#include "engine/render_camera.h"
#include "engine/voxel_chunk.h"

#include <queue>

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
		int MAX_CHUNKS_PER_FRAME_GENERATED = 16;

		int m_RenderDistance = 5;
		RenderCamera m_RenderCam;

		std::unordered_map<glm::vec3, std::shared_ptr<VoxelChunk>, KeyHasher, KeyEquals> m_TotalChunks;
		std::queue<std::shared_ptr<VoxelChunk>> m_RenderQueue;
		std::vector<std::shared_ptr<VoxelChunk>> m_RenderChunks;
	};
}
