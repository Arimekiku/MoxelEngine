#pragma once

#include <glm/glm.hpp>

namespace Moxel
{
	class RenderCamera
	{
	public:
		RenderCamera() = default;
		RenderCamera(glm::vec3 position, glm::vec3 orientation);
		~RenderCamera() = default;

		void update();
		void resize(uint16_t width, uint16_t height);
		void set_perspective(float fov, float minDist, float maxDist);

		glm::vec3 get_position() const { return m_position; }
		glm::mat4 get_proj_view_mat() const { return glm::mat4(m_proj * m_view); }
	private:
		void set_orientation(float rotX, float rotY);

		bool m_cameraMode = false;
		float m_fov = 45;
		float m_minRenderDist = 0.1f;
		float m_maxRenderDist = 1000.0f;
		float m_aspect = 1.0f;

		glm::vec3 m_position;
		glm::vec3 m_orientation;

		glm::mat4 m_view = glm::mat4(1.0f);
		glm::mat4 m_proj = glm::mat4(1.0f);
	};
}