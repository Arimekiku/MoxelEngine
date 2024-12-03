#include "image.h"
#include <iostream>

namespace SDLarria {
	Image::Image(const char* path, SDL_PixelFormat format) {
		m_ImageSurface = SDL_LoadBMP(path);
		if (m_ImageSurface == nullptr) {
			printf(SDL_GetError());
		}

		if (format != SDL_PIXELFORMAT_UNKNOWN) {
			SDL_Surface* new_surface = SDL_ConvertSurface(m_ImageSurface, format);

			if (new_surface) {
				SDL_DestroySurface(m_ImageSurface);
				m_ImageSurface = new_surface;
				return;
			}

			printf(SDL_GetError());
		}
	}

	Image::~Image() {
		SDL_DestroySurface(m_ImageSurface);
	}

	void Image::Render(SDL_Surface* target) {
		SDL_BlitSurface(m_ImageSurface, nullptr, target, nullptr);
	}
}