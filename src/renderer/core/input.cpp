#include "input.h"
#include "renderer/application.h"

namespace Moxel::Input
{
	std::unordered_map<uint16_t, bool> Key::m_keyMap = std::unordered_map<uint16_t, bool>(256);
	std::unordered_map<uint16_t, bool> Key::m_oldKeyMap = std::unordered_map<uint16_t, bool>(256);

	bool Key::just_pressed(const KeyCode code)
	{
		return m_keyMap[code] != m_oldKeyMap[code] && m_keyMap[code] == true;
	}

	bool Key::just_released(const KeyCode code)
	{
		return m_keyMap[code] != m_oldKeyMap[code] && m_keyMap[code] == false;
	}

	bool Key::pressed(const KeyCode code)
	{
		return m_keyMap[code] == true;
	}

	bool Key::released(const KeyCode code)
	{
		return m_keyMap[code] == false;
	}

	glm::vec2 Mouse::m_normalizedCursor;

	int Key::get_axis_value(const KeyCode positiveKey, const KeyCode negativeKey)
	{
		return pressed(positiveKey) - pressed(negativeKey);
	}

	bool Mouse::button_pressed(const int code)
	{
		// TODO: implement

		return false;
	}

	bool Mouse::button_released(const int code)
	{
		// TODO: implement

		return false;
	}

	void Mouse::set_cursor_mode(const CursorMode mode)
	{
		auto* window = Application::get().get_window().get_native_window();

		SDL_SetWindowRelativeMouseMode(window, mode);

		switch (mode)
		{
			case VISIBLE: SDL_ShowCursor(); break;
			case HIDDEN: SDL_HideCursor(); break;
			default: LOG_ASSERT(false, "Cursor mode is invalid!");
		}
	}

	glm::vec2 Mouse::se_get_cursor_position()
	{
		float mouseX, mouseY;
		SDL_GetMouseState(&mouseX, &mouseY);

		return { mouseX, mouseY };
	}

	void Mouse::set_cursor_position(const int x, const int y)
	{
		auto* window = Application::get().get_window().get_native_window();

		SDL_WarpMouseInWindow(window, x, y);
	}

	float Mouse::get_mouse_x()
	{
		float xPos, yPos;
		SDL_GetMouseState(&xPos, &yPos);

		return xPos;
	}

	float Mouse::get_mouse_y()
	{
		float xPos, yPos;
		SDL_GetMouseState(&xPos, &yPos);

		return yPos;
	}

	glm::vec2 Mouse::get_normalized_cursor()
	{
		float xPos, yPos;
		SDL_GetRelativeMouseState(&xPos, &yPos);

		return { xPos, yPos };
	}

	void Mouse::set_cursor_in_center_of_window()
	{
		const auto& [width, height] = Application::get().get_window().get_window_size();

		set_cursor_position(width / 2, height / 2);
	}
}
