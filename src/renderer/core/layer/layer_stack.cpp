#include "layer_stack.h"

#include <algorithm>

namespace Moxel
{
    void LayerStack::Push(Layer* layer)
    {
        m_Layers.emplace_back(layer);
    }

    void LayerStack::Pop(const Layer* layer)
    {
        const auto iterator = std::ranges::find(m_Layers, layer);

        if (iterator != m_Layers.end())
        {
            m_Layers.erase(iterator);
        }
    }

    void LayerStack::Clear()
    {
        for (Layer* layer : m_Layers)
        {
            layer->Detach();
            delete layer;
        }
    }
}
