#pragma once

#include "renderer/vulkan/vulkan_buffer_vertex_array.h"

#include <memory>

namespace Moxel
{
	class RenderMesh
	{
	public:
		RenderMesh(const std::shared_ptr<VulkanVertexArray>& vertexArray);
		~RenderMesh();

		void set_position(glm::vec3 position);
		void set_rotation(glm::vec3 rotation);
		void set_scale(glm::vec3 scale);

		const std::shared_ptr<VulkanVertexArray>& get_vertex_array() const;
		glm::mat4 get_trs_matrix() const;

	private:
		std::shared_ptr<VulkanVertexArray> m_vertexArray;

		glm::vec3 m_position = glm::vec3(0.0f);
		glm::vec3 m_rotation = glm::vec3(0.0f);
		glm::vec3 m_scale = glm::vec3(1.0f);
	};
}
