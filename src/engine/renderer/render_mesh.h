#pragma once

#include "engine/vulkan/vulkan_buffer_vertex_array.h"

#include <memory>

namespace SDLarria
{
	class RenderMesh
	{
	public:
		RenderMesh(const std::shared_ptr<VulkanVertexArray>& vertexArray);
		~RenderMesh();

		void SetPosition(glm::vec3 position);
		void SetRotation(glm::vec3 rotation);
		void SetScale(glm::vec3 scale);

		const std::shared_ptr<VulkanVertexArray>& GetVertexArray() const;
		glm::mat4 GetTRSMatrix() const;

	private:
		std::shared_ptr<VulkanVertexArray> m_Mesh;

		glm::vec3 m_Position = glm::vec3(0.0f);
		glm::vec3 m_Rotation = glm::vec3(0.0f);
		glm::vec3 m_Scale = glm::vec3(1.0f);
	};
}
