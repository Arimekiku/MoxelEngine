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
		m_Mesh = vertexArray;
	}

	RenderMesh::~RenderMesh()
	{

	}

	void RenderMesh::SetPosition(const glm::vec3 position)
	{
		m_Position = position;
	}

	void RenderMesh::SetRotation(const glm::vec3 rotation)
	{
		m_Rotation = rotation;
	}

	void RenderMesh::SetScale(const glm::vec3 scale)
	{
		m_Scale = scale;
	}

	const std::shared_ptr<VulkanVertexArray>& RenderMesh::GetVertexArray() const
	{
		return m_Mesh;
	}

	glm::mat4 RenderMesh::GetTRSMatrix() const
	{
		const glm::mat4 translation = translate(glm::mat4(1.0f), m_Position);
		const glm::mat4 rotation = toMat4(glm::quat(radians(m_Rotation)));
		const glm::mat4 scaling = scale(glm::mat4(1.0f), m_Scale);

		return translation * rotation * scaling;
	}
}
