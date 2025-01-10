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

		void Update();
		void Resize(uint16_t width, uint16_t height);
		void SetPerspective(float fov, float minDist, float maxDist);

		glm::vec3 GetPosition() const { return m_Position; }
		glm::mat4 GetProjViewMat() const { return glm::mat4(m_Proj * m_View); }
	private:
		void SetOrientation(float rotX, float rotY);

		bool m_CameraMode = false;
		float m_FOV = 45;
		float m_MinRenderDist = 0.1f;
		float m_MaxRenderDist = 1000.0f;
		float m_Aspect = 1.0f;

		glm::vec3 m_Position;
		glm::vec3 m_Orientation;

		glm::mat4 m_View = glm::mat4(1.0f);
		glm::mat4 m_Proj = glm::mat4(1.0f);
	};
}