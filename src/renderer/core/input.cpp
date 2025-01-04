#include "input.h"
#include "renderer/application.h"

namespace Moxel::Input
{
	std::unordered_map<uint16_t, bool> Key::m_KeyMap = std::unordered_map<uint16_t, bool>(256);
	std::unordered_map<uint16_t, bool> Key::m_OldKeyMap = std::unordered_map<uint16_t, bool>(256);

	bool Key::JustPressed(const KeyCode code)
	{
		return m_KeyMap[code] != m_OldKeyMap[code] && m_KeyMap[code] == true;
	}

	bool Key::JustReleased(const KeyCode code)
	{
		return m_KeyMap[code] != m_OldKeyMap[code] && m_KeyMap[code] == false;
	}

	bool Key::Pressed(const KeyCode code)
	{
		return m_KeyMap[code] == true;
	}

	bool Key::Released(const KeyCode code)
	{
		return m_KeyMap[code] == false;
	}

	glm::vec2 Mouse::m_NormalizedCursor;

	int Key::GetAxisValue(const KeyCode positiveKey, const KeyCode negativeKey)
	{
		return Pressed(positiveKey) - Pressed(negativeKey);
	}

	bool Mouse::ButtonPressed(const int code)
	{
		// TODO: implement

		return false;
	}

	bool Mouse::ButtonReleased(const int code)
	{
		// TODO: implement

		return false;
	}

	void Mouse::SetCursorMode(const CursorMode mode)
	{
		auto* window = Application::Get().GetWindow().GetNativeWindow();

		SDL_SetWindowRelativeMouseMode(window, mode);

		switch (mode)
		{
			case VISIBLE: SDL_ShowCursor(); break;
			case HIDDEN: SDL_HideCursor(); break;
			default: LOG_ASSERT(false, "Cursor mode is invalid!");
		}
	}

	glm::vec2 Mouse::GetCursorPosition()
	{
		float mouseX, mouseY;
		SDL_GetMouseState(&mouseX, &mouseY);

		return { mouseX, mouseY };
	}

	void Mouse::SetCursorPosition(const int x, const int y)
	{
		auto* window = Application::Get().GetWindow().GetNativeWindow();

		SDL_WarpMouseInWindow(window, x, y);
	}

	float Mouse::GetMouseX()
	{
		float xPos, yPos;
		SDL_GetMouseState(&xPos, &yPos);

		return xPos;
	}

	float Mouse::GetMouseY()
	{
		float xPos, yPos;
		SDL_GetMouseState(&xPos, &yPos);

		return yPos;
	}

	glm::vec2 Mouse::GetNormalizedCursor()
	{
		float xPos, yPos;
		SDL_GetRelativeMouseState(&xPos, &yPos);

		return { xPos, yPos };
	}

	void Mouse::SetCursorInCenterOfWindow()
	{
		const auto& [width, height] = Application::Get().GetWindow().GetWindowSize();

		SetCursorPosition(width / 2, height / 2);
	}
}
