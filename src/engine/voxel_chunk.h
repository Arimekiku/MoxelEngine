#pragma once

#include "render_mesh.h"

#include <memory>

namespace Moxel
{
	class VoxelChunk
	{
	public:
		VoxelChunk();

		const std::shared_ptr<RenderMesh>& GetVertexArray() const { return m_VertexArray; }

	private:
		std::shared_ptr<RenderMesh> m_VertexArray;

		bool m_Blocks[10][10][10] = { };
	};
}
