#pragma once
#include "renderer/vulkan/vulkan_buffer_vertex_array.h"

namespace Moxel
{
	enum class Direction
	{
		Forward,
		Backward,
		Left,
		Right,
		Up,
		Down
	};

	class RenderQuad
	{
	public:
		RenderQuad(Direction direction, glm::vec3 position);

		void AddIndicesOffset(int value);

		const std::vector<Vertex>& GetVertices() const { return m_Vertices; }
		const std::vector<uint32_t>& GetIndices() const { return m_Indices; }

	private:
		std::vector<Vertex> m_Vertices;
		std::vector<uint32_t> m_Indices;
	};
}
