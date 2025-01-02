#include "render_camera.h"
#include "engine/application.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/trigonometric.hpp>
#include <glm/gtx/vector_angle.hpp>

#include "engine/core/input.h"

namespace SDLarria
{
	void RenderCamera::SetOrientation(const float rotX, const float rotY)
	{
		// Calculates upcoming vertical change in the Orientation
		const glm::vec3 newOrientation = rotate(m_Orientation, glm::radians(-rotX),
													 normalize(cross(m_Orientation, glm::vec3(0, 1, 0))));

		// Decides whether the next vertical Orientation is legal or not
		if (std::abs(angle(newOrientation, glm::vec3(0, 1, 0)) - glm::radians(90.0f)) <= glm::radians(85.0f))
		{
			m_Orientation = newOrientation;
		}

		// Rotates the Orientation left and right
		m_Orientation = rotate(m_Orientation, glm::radians(-rotY), glm::vec3(0, 1, 0));
	}

	RenderCamera::RenderCamera(const glm::vec3 position, const glm::vec3 orientation)
	{
		const auto& [width, height] = Application::Get().GetWindow().GetWindowSize();

		m_Position = position;
		m_Orientation = orientation;
		m_View = lookAt(m_Position, m_Position + m_Orientation, glm::vec3(0, 1, 0));

		Resize(width, height);
	}

	void RenderCamera::Resize(const uint16_t width, const uint16_t height)
	{
		m_Aspect = (float)width / (float)height;

		m_Proj = glm::perspective(glm::radians(m_FOV), m_Aspect, m_MinRenderDist, m_MaxRenderDist);
		m_Proj[1][1] *= -1;
	}

	void RenderCamera::Update()
	{
		if (Input::Key::JustPressed(Input::Key::C))
		{
			m_CameraMode = !m_CameraMode;
			//Input::Mouse::SetCursorInCenterOfWindow();
			Input::Mouse::SetCursorMode(m_CameraMode ? Input::Mouse::CursorMode::VISIBLE : Input::Mouse::CursorMode::HIDDEN);
		}

		if (m_CameraMode == false)
		{
			return;
		}

		const glm::vec2 rotationVector = Input::Mouse::GetNormalizedCursor();
		SetOrientation(rotationVector.y / 10, rotationVector.x / 10);
		//Input::Mouse::SetCursorInCenterOfWindow();

		const int horizontalAxis = Input::Key::GetAxisValue(Input::Key::D, Input::Key::A);
		m_Position += static_cast<float>(horizontalAxis) * normalize(cross(m_Orientation, glm::vec3(0, 1, 0)));

		const int verticalAxis = Input::Key::GetAxisValue(Input::Key::W, Input::Key::S);
		m_Position += static_cast<float>(verticalAxis) * m_Orientation;

		m_View = lookAt(m_Position, m_Position + m_Orientation, glm::vec3(0, 1, 0));
	}

	void RenderCamera::SetPerspective(const float fov, const float minDist, const float maxDist)
	{
		m_FOV = fov;
		m_MinRenderDist = minDist;
		m_MaxRenderDist = maxDist;

		m_Proj = glm::perspective(glm::radians(m_FOV), m_Aspect, m_MinRenderDist, m_MaxRenderDist);
	}
}
