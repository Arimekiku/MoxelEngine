#pragma once
#include "renderer/vulkan/vulkan_buffer_vertex_array.h"

namespace Moxel
{
	enum class Side
	{
		//TODO: currently we only need 3 sides, think about it
		Front,
		Back,
		Left,
		Right,
		Up,
		Down
	};

	class RenderQuad
	{
	public:
		RenderQuad(Side side, glm::vec3 position);

		void add_indices_offset(int value);

		const std::vector<Vertex>& get_vertices() const { return m_vertices; }
		const std::vector<uint32_t>& get_indices() const { return m_indices; }

	private:
		std::vector<Vertex> m_vertices;
		std::vector<uint32_t> m_indices;
	};
}
