#include "window.h"
#include "engine/ui/gui_layer.h"

#include <iostream>
#include <imgui.h>

namespace SDLarria 
{
	GameWindow::GameWindow(const int width, const int height)
    {
        SDL_Init(SDL_INIT_VIDEO);

        constexpr SDL_WindowFlags window_flags = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE;

	    m_WindowSize = VkExtent2D(width, height);
        m_NativeWindow = SDL_CreateWindow("Vulkan Engine", m_WindowSize.width, m_WindowSize.height, window_flags);
        if (m_NativeWindow == nullptr) 
        {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create window: %s\n", SDL_GetError());
        }
	}

    GameWindow::~GameWindow()
    {
        SDL_DestroyWindow(m_NativeWindow);
        SDL_Quit();
    }

    void GameWindow::Update(VulkanRenderer& renderer, LayerStack layers)
    {
        bool is_open = true;

        while (is_open) 
        {
            SDL_Event event;
            while (SDL_PollEvent(&event)) 
            {
                if (event.type == SDL_EVENT_QUIT)
                {
                    is_open = false;
                    break;
                }

                if (event.type == SDL_EVENT_KEY_DOWN) 
                {
                    if (event.key.key == SDLK_ESCAPE) 
                    {
                        is_open = false;
                        break;
                    }
                }

                GuiLayer::ProcessEvents(event);
            }

            for (const auto layer : layers)
            {
                layer->OnEveryUpdate();
            }

            GuiLayer::Begin();

            for (const auto layer : layers)
            {
                layer->OnGuiUpdate();
            }

            ImGui::ShowDemoWindow();

            if (ImGui::Begin("background"))
            {
                auto& [data1, data2, data3, data4] = VulkanRenderer::Get().GetShader_TEST()->GetPushConstants();

                ImGui::InputFloat4("data1",(float*)&data1);
                ImGui::InputFloat4("data2",(float*)&data2);
                ImGui::InputFloat4("data3",(float*)&data3);
                ImGui::InputFloat4("data4",(float*)&data4);
            }
            ImGui::End();

            GuiLayer::End();

            renderer.Draw();
        }
    }

    void GameWindow::Resize()
    {
	    SDL_GetWindowSizeInPixels(m_NativeWindow, (int*)&m_WindowSize.width, (int*)&m_WindowSize.height);
    }
}