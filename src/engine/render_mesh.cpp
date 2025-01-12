#include "render_mesh.h"

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Moxel
{
	RenderMesh::RenderMesh(const std::shared_ptr<VulkanVertexArray>& vertexArray)
	{
		m_vertexArray = vertexArray;
	}

	RenderMesh::~RenderMesh()
	{

	}

	void RenderMesh::set_position(const glm::vec3 position)
	{
		m_position = position;
	}

	void RenderMesh::set_rotation(const glm::vec3 rotation)
	{
		m_rotation = rotation;
	}

	void RenderMesh::set_scale(const glm::vec3 scale)
	{
		m_scale = scale;
	}

	const std::shared_ptr<VulkanVertexArray>& RenderMesh::get_vertex_array() const
	{
		return m_vertexArray;
	}

	glm::mat4 RenderMesh::get_trs_matrix() const
	{
		const glm::mat4 translation = translate(glm::mat4(1.0f), m_position);
		const glm::mat4 rotation = toMat4(glm::quat(radians(m_rotation)));
		const glm::mat4 scaling = scale(glm::mat4(1.0f), m_scale);

		return translation * rotation * scaling;
	}
}
