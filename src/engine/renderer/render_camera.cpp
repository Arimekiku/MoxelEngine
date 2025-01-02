#include "render_camera.h"
#include "engine/application.h"

#include <glm/gtx/vector_angle.hpp>

namespace SDLarria
{
	RenderCamera::RenderCamera(glm::vec3 position, glm::vec3 orientation)
	{
		const auto& window = Application::Get().GetWindow().GetWindowSize();

		m_Position = position;
		m_Orientation = orientation;
		m_View = glm::lookAt(m_Position, m_Position + m_Orientation, glm::vec3(0, 0, 1));

		Resize(window.width, window.height);
	}

	void RenderCamera::Resize(const uint16_t width, const uint16_t height)
	{
		m_Aspect = (float)width / (float)height;

		m_Proj = glm::perspective(glm::radians(m_FOV), m_Aspect, m_MinRenderDist, m_MaxRenderDist);
		m_Proj[1][1] *= -1;
	}

	void RenderCamera::SetPerspective(float fov, float minDist, float maxDist)
	{
		m_FOV = fov;
		m_MinRenderDist = minDist;
		m_MaxRenderDist = maxDist;

		m_Proj = glm::perspective(glm::radians(m_FOV), m_Aspect, m_MinRenderDist, m_MaxRenderDist);
	}
}