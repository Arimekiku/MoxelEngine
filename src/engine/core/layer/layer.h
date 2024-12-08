#pragma once


namespace SDLarria
{
    class Layer
    {
    public:
        Layer() = default;
        virtual ~Layer() = default;

        virtual void Attach() { }
        virtual void Detach() { }
        virtual void OnEveryUpdate() { }
        virtual void OnGuiUpdate() { }
    };
}