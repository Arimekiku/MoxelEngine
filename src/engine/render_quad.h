#pragma once

#include <glm/glm.hpp>

namespace Moxel
{
	struct VoxelVertex
	{
		uint32_t Position;
		glm::vec3 Color;

		VoxelVertex() = default;
		VoxelVertex(const glm::u8vec3 localCoord, const glm::vec3 color) 
		{
			Color = color;
			Position = static_cast<uint32_t>(localCoord.x) | static_cast<uint32_t>(localCoord.y) << 5 | static_cast<uint32_t>(localCoord.z) << 10;
		}
	};

	enum class Side
	{
		FRONT,
		BACK,
		LEFT,
		RIGHT,
		UP,
		DOWN
	};

	class RenderQuad
	{
	public:
		RenderQuad(Side side, glm::u8vec3 position);

		void add_indices_offset(int value);

		const std::vector<VoxelVertex>& get_vertices() const { return m_vertices; }
		const std::vector<uint32_t>& get_indices() const { return m_indices; }

	private:
		std::vector<VoxelVertex> m_vertices;
		std::vector<uint32_t> m_indices;
	};
}
