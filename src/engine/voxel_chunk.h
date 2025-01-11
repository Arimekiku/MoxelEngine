#pragma once

#include "render_mesh.h"

#include <memory>

namespace Moxel
{
	class VoxelChunk
	{
	public:
		VoxelChunk(glm::vec3 position);
		~VoxelChunk();

		void SetForDeletion() { m_QueuedForDeletion = true; }
		bool IsSetForDeletion() const { return m_QueuedForDeletion; }
		bool IsEmpty() const { return m_VertexArray == nullptr || m_VertexArray->GetVertexArray()->IsEmpty(); }

		void RecreateChunk();

		glm::vec3 GetPosition() const { return m_Position; }
		const std::shared_ptr<RenderMesh>& GetVertexArray() const { return m_VertexArray; }

	private:
		bool m_QueuedForDeletion = false;

		std::shared_ptr<RenderMesh> m_VertexArray = nullptr;
		glm::vec3 m_Position = glm::vec3(0);

		bool m_Blocks[16][16][16];
	};
}
