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

			static void SetKeyValue(const uint16_t code, const bool value) { m_KeyMap[code] = value; }
			static void CopyNewLayout() { m_OldKeyMap = m_KeyMap; }

			static bool JustPressed(KeyCode code);
			static bool JustReleased(KeyCode code);
			static bool Pressed(KeyCode code);
			static bool Released(KeyCode code);

			static int GetAxisValue(KeyCode positiveKey, KeyCode negativeKey);

		private:
			static std::unordered_map<uint16_t, bool> m_KeyMap;
			static std::unordered_map<uint16_t, bool> m_OldKeyMap;
		};

		class Mouse
		{
		public:
			enum CursorMode
			{
				VISIBLE = true,
				HIDDEN = false,
			};

			static bool ButtonPressed(int code);
			static bool ButtonReleased(int code);

			static void SetMouseRelative(const int x, const int y) { m_NormalizedCursor.x = x; m_NormalizedCursor.y = y; }
			static void SetCursorMode(CursorMode mode);

			static glm::vec2 GetCursorPosition();
			static glm::vec2 GetNormalizedCursor();

			static void SetCursorPosition(int x, int y);

			static float GetMouseX();
			static float GetMouseY();

			static void SetCursorInCenterOfWindow();

		private:
			static glm::vec2 m_NormalizedCursor;
		};
	};
}
