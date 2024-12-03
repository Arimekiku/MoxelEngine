#include "window.h"
#include "image.h"
#include <iostream>

namespace SDLarria {
	GameWindow::GameWindow() {
		SDL_Init(SDL_INIT_VIDEO);

        m_NativeWindow = SDL_CreateWindow("SDL3", 640, 480, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
        if (m_NativeWindow == nullptr) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create window: %s\n", SDL_GetError());
            return;
        }

        m_WindowSurface = SDL_GetWindowSurface(m_NativeWindow);
	}

    GameWindow::~GameWindow()
    {
        SDL_DestroyWindow(m_NativeWindow);
        SDL_Quit();
    }

    void GameWindow::Update()
    {
        bool is_open = true;

        Image new_image = Image(RESOURCES_PATH "test.bmp", m_WindowSurface->format);

        while (is_open) {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_EVENT_QUIT) {
                    is_open = false;
                }

                if (event.type == SDL_EVENT_KEY_DOWN) {
                    if (event.key.key == SDLK_ESCAPE) {
                        is_open = false;
                    }
                }

                if (event.type == SDL_EVENT_WINDOW_RESIZED) {
                    m_WindowSurface = SDL_GetWindowSurface(m_NativeWindow);
                }
            }

            auto details = SDL_GetPixelFormatDetails(m_WindowSurface->format);
            SDL_FillSurfaceRect(m_WindowSurface, nullptr, SDL_MapRGB(details, nullptr, 50, 50, 50));

            Uint64 Start{ SDL_GetPerformanceCounter() };
            new_image.Render(m_WindowSurface);
            Uint64 Delta{ SDL_GetPerformanceCounter() - Start };
            std::cout << "\nTime to Render Image: " << Delta;

            SDL_UpdateWindowSurface(m_NativeWindow);
        }
    }
}