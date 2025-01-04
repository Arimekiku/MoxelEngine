#pragma once

#include "layer.h"

#include <vector>

namespace Moxel
{
    class LayerStack
    {
    public:
        LayerStack() = default;
        ~LayerStack() = default;

        void Push(Layer* layer);
        void Pop(const Layer* layer);
        void Clear();

        std::vector<Layer*>::iterator begin() { return m_Layers.begin(); }
        std::vector<Layer*>::iterator end() { return m_Layers.end(); }

    private:
        std::vector<Layer*> m_Layers;
    };
}