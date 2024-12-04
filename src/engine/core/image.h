#pragma once

#include "SDL3/SDL.h"

namespace SDLarria {
	class Image {
	public:
		Image(const char* path, SDL_PixelFormat format);
		~Image();

		void Render(SDL_Surface* target);

	private:
		SDL_Surface* m_ImageSurface = nullptr;
	};
}