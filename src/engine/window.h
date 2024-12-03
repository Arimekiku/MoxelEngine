#include <SDL3/SDL.h>

namespace SDLarria {
	class GameWindow {
	public:
		GameWindow();
		~GameWindow();

		const SDL_Surface* GetWindowSurface() { return m_WindowSurface; }

		void Update();

	private:
		SDL_Window* m_NativeWindow = nullptr;
		SDL_Surface* m_WindowSurface = nullptr;
	};
}