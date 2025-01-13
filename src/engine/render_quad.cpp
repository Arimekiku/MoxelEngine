#include "render_quad.h"
#include "renderer/core/logger/log.h"

namespace Moxel
{
	RenderQuad::RenderQuad(const Side side, const glm::vec3 position)
	{
		m_indices = std::vector<uint32_t> { 0, 1, 2, 0, 2, 3 };
		m_vertices.resize(4);

		switch (side)
		{
			case Side::Down:
			{
				m_vertices[0] = Vertex({ -0.5f, -0.5f, -0.5f }, { 1, 0, 0 });
				m_vertices[1] = Vertex({ 0.5f, -0.5f, -0.5f }, { 0, 1, 0 });
				m_vertices[2] = Vertex({ 0.5f, -0.5f, 0.5f }, { 0, 0, 1 });
				m_vertices[3] = Vertex({ -0.5f, -0.5f, 0.5f }, { 1, 1, 1 });

				break;
			}
			case Side::Up:
			{
				m_vertices[0] = Vertex({ -0.5f, 0.5f, 0.5f }, { 1, 0, 0 });
				m_vertices[1] = Vertex({ 0.5f, 0.5f, 0.5f }, { 0, 1, 0 });
				m_vertices[2] = Vertex({ 0.5f, 0.5f, -0.5f }, { 0, 0, 1 });
				m_vertices[3] = Vertex({ -0.5f, 0.5f, -0.5f }, { 1, 1, 1 });

				break;
			}
			case Side::Left:
			{
				m_vertices[0] = Vertex({ -0.5f, -0.5f, -0.5f }, { 1, 0, 0 });
				m_vertices[1] = Vertex({ -0.5f, -0.5f, 0.5f }, { 0, 1, 0 });
				m_vertices[2] = Vertex({ -0.5f, 0.5f, 0.5f }, { 0, 0, 1 });
				m_vertices[3] = Vertex({ -0.5f, 0.5f, -0.5f }, { 1, 1, 1 });

				break;
			}
			case Side::Right:
			{
				m_vertices[0] = Vertex({ 0.5f, 0.5f, -0.5f }, { 1, 0, 0 });
				m_vertices[1] = Vertex({ 0.5f, 0.5f, 0.5f }, { 0, 1, 0 });
				m_vertices[2] = Vertex({ 0.5f, -0.5f, 0.5f }, { 0, 0, 1 });
				m_vertices[3] = Vertex({ 0.5f, -0.5f, -0.5f }, { 1, 1, 1 });

				break;
			}
			case Side::Front:
			{
				m_vertices[0] = Vertex({ 0.5f, -0.5f, 0.5f }, { 1, 0, 0 });
				m_vertices[1] = Vertex({ 0.5f, 0.5f, 0.5f }, { 0, 1, 0 });
				m_vertices[2] = Vertex({ -0.5f, 0.5f, 0.5f }, { 0, 0, 1 });
				m_vertices[3] = Vertex({ -0.5f, -0.5f, 0.5f }, { 1, 1, 1 });

				break;
			}
			case Side::Back:
			{
				m_vertices[0] = Vertex({ -0.5f, -0.5f, -0.5f }, { 1, 0, 0 });
				m_vertices[1] = Vertex({ -0.5f, 0.5f, -0.5f }, { 0, 1, 0 });
				m_vertices[2] = Vertex({ 0.5f, 0.5f, -0.5f }, { 0, 0, 1 });
				m_vertices[3] = Vertex({ 0.5f, -0.5f, -0.5f }, { 1, 1, 1 });

				break;
			}
			default: LOG_ASSERT(false, "Invalid direction");
		}

		for (auto& [Position, _] : m_vertices)
		{
			Position += position;
		}
	}

	void RenderQuad::add_indices_offset(const int value)
	{
		for (uint32_t& index : m_indices)
		{
			index += value;
		}
	}
}
