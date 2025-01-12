#pragma once

#include <unordered_map>
#include <SDL3/SDL_keycode.h>
#include <glm/vec2.hpp>

namespace Moxel
{
	namespace Input
	{
		class Key
		{
		public:
			enum KeyCode
			{
				W = SDLK_W,
				O = SDLK_O,
				A = SDLK_A,
				S = SDLK_S,
				D = SDLK_D,
				C = SDLK_C,
				N = SDLK_N,
				F1 = SDLK_F1,
				F2 = SDLK_F2,
				F3 = SDLK_F3,
				LEFT_SHIFT = SDLK_LSHIFT,
				RIGHT_SHIFT = SDLK_RSHIFT,
				LEFT_CTRL = SDLK_LCTRL,
				RIGHT_CTRL = SDLK_RCTRL,
				SPACE = SDLK_SPACE,
				ESCAPE = SDLK_ESCAPE,
			};

			static void set_key_value(const uint16_t code, const bool value) { m_keyMap[code] = value; }
			static void copy_new_layout() { m_oldKeyMap = m_keyMap; }

			static bool just_pressed(KeyCode code);
			static bool just_released(KeyCode code);
			static bool pressed(KeyCode code);
			static bool released(KeyCode code);

			static int get_axis_value(KeyCode positiveKey, KeyCode negativeKey);

		private:
			static std::unordered_map<uint16_t, bool> m_keyMap;
			static std::unordered_map<uint16_t, bool> m_oldKeyMap;
		};

		class Mouse
		{
		public:
			enum CursorMode
			{
				VISIBLE = true,
				HIDDEN = false,
			};

			static bool button_pressed(int code);
			static bool button_released(int code);

			static void set_mouse_relative(const int x, const int y) { m_normalizedCursor.x = x; m_normalizedCursor.y = y; }
			static void set_cursor_mode(CursorMode mode);

			static glm::vec2 se_get_cursor_position();
			static glm::vec2 get_normalized_cursor();

			static void set_cursor_position(int x, int y);

			static float get_mouse_x();
			static float get_mouse_y();

			static void set_cursor_in_center_of_window();

		private:
			static glm::vec2 m_normalizedCursor;
		};
	};
}
