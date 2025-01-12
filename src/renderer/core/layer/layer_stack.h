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

		void push(Layer* layer);
		void pop(const Layer* layer);
		void clear();

		std::vector<Layer*>::iterator begin() { return m_layers.begin(); }
		std::vector<Layer*>::iterator end() { return m_layers.end(); }

	private:
		std::vector<Layer*> m_layers;
	};
}