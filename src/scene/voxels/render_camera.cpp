#include "render_camera.h"
#include "engine/application.h"
#include "engine/core/input.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/trigonometric.hpp>
#include <glm/gtx/vector_angle.hpp>

namespace Moxel
{
	void RenderCamera::set_orientation(const float rotX, const float rotY)
	{
		// Calculates upcoming vertical change in the Orientation
		const glm::vec3 newOrientation = rotate(m_orientation, glm::radians(-rotX),
													 normalize(cross(m_orientation, glm::vec3(0, 1, 0))));

		// Decides whether the next vertical Orientation is legal or not
		if (std::abs(angle(newOrientation, glm::vec3(0, 1, 0)) - glm::radians(90.0f)) <= glm::radians(85.0f))
			m_orientation = newOrientation;

		// Rotates the Orientation left and right
		m_orientation = rotate(m_orientation, glm::radians(-rotY), glm::vec3(0, 1, 0));
	}

	RenderCamera::RenderCamera(const glm::vec3 position, const glm::vec3 orientation)
	{
		const auto& [width, height] = Application::get().get_window().get_window_size();

		m_position = position;
		m_orientation = orientation;
		m_view = lookAt(m_position, m_position + m_orientation, glm::vec3(0, 1, 0));

		resize(width, height);
	}

	void RenderCamera::resize(const uint16_t width, const uint16_t height)
	{
		m_aspect = static_cast<float>(width) / static_cast<float>(height);

		m_proj = glm::perspective(glm::radians(m_fov), m_aspect, m_minRenderDist, m_maxRenderDist);
		m_proj[1][1] *= -1;
	}

	void RenderCamera::update()
	{
		if (Input::Key::just_pressed(Input::Key::C))
		{
			m_cameraMode = !m_cameraMode;
			Input::Mouse::set_cursor_mode(m_cameraMode ? Input::Mouse::CursorMode::VISIBLE : Input::Mouse::CursorMode::HIDDEN);
		}

		if (m_cameraMode == false)
			return;

		const glm::vec2 rotationVector = Input::Mouse::get_normalized_cursor();
		set_orientation(rotationVector.y / 10, rotationVector.x / 10);

		const int horizontalAxis = Input::Key::get_axis_value(Input::Key::D, Input::Key::A);
		m_position += static_cast<float>(horizontalAxis) * normalize(cross(m_orientation, glm::vec3(0, 1, 0)));

		const int verticalAxis = Input::Key::get_axis_value(Input::Key::W, Input::Key::S);
		m_position += static_cast<float>(verticalAxis) * m_orientation;

		m_view = lookAt(m_position, m_position + m_orientation, glm::vec3(0, 1, 0));
	}

	void RenderCamera::set_perspective(const float fov, const float minDist, const float maxDist)
	{
		m_fov = fov;
		m_minRenderDist = minDist;
		m_maxRenderDist = maxDist;

		m_proj = glm::perspective(glm::radians(m_fov), m_aspect, m_minRenderDist, m_maxRenderDist);
	}
}
