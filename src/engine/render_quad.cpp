#include "render_quad.h"
#include "renderer/core/logger/log.h"

namespace Moxel
{
	RenderQuad::RenderQuad(const Side side, const glm::u8vec3 position)
	{
		m_indices = std::vector<uint32_t> { 0, 1, 2, 0, 2, 3 };
		m_vertices.resize(4);

		switch (side)
		{
			case Side::DOWN:
			{
				m_vertices[0] = VoxelVertex({0 + position.x, 0 + position.y, 0 + position.z}, {1, 0, 0});
				m_vertices[1] = VoxelVertex({1 + position.x, 0 + position.y, 0 + position.z}, {0, 1, 0});
				m_vertices[2] = VoxelVertex({1 + position.x, 0 + position.y, 1 + position.z}, {0, 0, 1});
				m_vertices[3] = VoxelVertex({0 + position.x, 0 + position.y, 1 + position.z}, {1, 1, 1});

				break;
			}
			case Side::UP:
			{
				m_vertices[0] = VoxelVertex({0 + position.x, 1 + position.y, 1 + position.z}, {1, 0, 0});
				m_vertices[1] = VoxelVertex({1 + position.x, 1 + position.y, 1 + position.z}, {0, 1, 0});
				m_vertices[2] = VoxelVertex({1 + position.x, 1 + position.y, 0 + position.z}, {0, 0, 1});
				m_vertices[3] = VoxelVertex({0 + position.x, 1 + position.y, 0 + position.z}, {1, 1, 1});

				break;
			}
			case Side::LEFT:
			{
				m_vertices[0] = VoxelVertex({0 + position.x, 0 + position.y, 0 + position.z}, {1, 0, 0});
				m_vertices[1] = VoxelVertex({0 + position.x, 0 + position.y, 1 + position.z}, {0, 1, 0});
				m_vertices[2] = VoxelVertex({0 + position.x, 1 + position.y, 1 + position.z}, {0, 0, 1});
				m_vertices[3] = VoxelVertex({0 + position.x, 1 + position.y, 0 + position.z}, {1, 1, 1});

				break;
			}
			case Side::RIGHT: 
			{
				m_vertices[0] = VoxelVertex({1 + position.x, 1 + position.y, 0 + position.z}, {1, 0, 0});
				m_vertices[1] = VoxelVertex({1 + position.x, 1 + position.y, 1 + position.z}, {0, 1, 0});
				m_vertices[2] = VoxelVertex({1 + position.x, 0 + position.y, 1 + position.z}, {0, 0, 1});
				m_vertices[3] = VoxelVertex({1 + position.x, 0 + position.y, 0 + position.z}, {1, 1, 1});

				break;
			}
			case Side::BACK:
			{
				m_vertices[0] = VoxelVertex({0 + position.x, 0 + position.y, 0 + position.z}, {1, 0, 0});
				m_vertices[1] = VoxelVertex({0 + position.x, 1 + position.y, 0 + position.z}, {0, 1, 0});
				m_vertices[2] = VoxelVertex({1 + position.x, 1 + position.y, 0 + position.z}, {0, 0, 1});
				m_vertices[3] = VoxelVertex({1 + position.x, 0 + position.y, 0 + position.z}, {1, 1, 1});

				break;
			}
			case Side::FRONT:
			{
				m_vertices[0] = VoxelVertex({1 + position.x, 0 + position.y, 1 + position.z}, {1, 0, 0});
				m_vertices[1] = VoxelVertex({1 + position.x, 1 + position.y, 1 + position.z}, {0, 1, 0});
				m_vertices[2] = VoxelVertex({0 + position.x, 1 + position.y, 1 + position.z}, {0, 0, 1});
				m_vertices[3] = VoxelVertex({0 + position.x, 0 + position.y, 1 + position.z}, {1, 1, 1});

				break;
			}
			default: LOG_ASSERT(false, "Invalid direction");
		}
	}

	void RenderQuad::add_indices_offset(const int value)
	{
		for (int i = 0; i < m_indices.size(); i++)
		{
			m_indices[i] += value;
		}
	}
}