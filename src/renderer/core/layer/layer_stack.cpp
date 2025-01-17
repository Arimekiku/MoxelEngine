#include "layer_stack.h"

#include <algorithm>

namespace Moxel
{
	void LayerStack::push(Layer* layer)
	{
		m_layers.emplace_back(layer);
	}

	void LayerStack::pop(const Layer* layer)
	{
		const auto iterator = std::ranges::find(m_layers, layer);

		if (iterator != m_layers.end())
		{
			m_layers.erase(iterator);
		}
	}

	void LayerStack::clear()
	{
		std::ranges::reverse(m_layers);
		for (const auto layer : m_layers)
		{
			layer->detach();
		}
		m_layers.clear();
	}
}