#pragma once

#include "engine/core/layer/layer.h"

#include <SDL3/SDL_events.h>

namespace SDLarria
{
    class GuiLayer final : public Layer
    {
    public:
        GuiLayer() = default;
        ~GuiLayer() override = default;

        void Attach() override;
        void Detach() override;

        static void ProcessEvents(const SDL_Event& event);
        static void Begin();
        static void End();
    };
}