#pragma once

#include "render_mesh.h"

#include <memory>

namespace Moxel
{
	class VoxelChunk
	{
	public:
		VoxelChunk() = default;
		VoxelChunk(int seed);
		~VoxelChunk();

		bool IsFree() const { return m_VertexArray == nullptr; }
		void Clear() { m_VertexArray = nullptr; }
		void RecreateChunk(glm::vec3 position);

		glm::vec3 GetPosition() const { return m_Position; }
		const std::shared_ptr<RenderMesh>& GetVertexArray() const { return m_VertexArray; }

	private:
		int m_Seed;
		std::shared_ptr<RenderMesh> m_VertexArray = nullptr;
		glm::vec3 m_Position = glm::vec3(0);

		bool m_Blocks[10][10][10] = { };
	};
}
