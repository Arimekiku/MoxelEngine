#pragma once

#include "core/core.h"
#include "vulkan/vulkan_context.h"

namespace Moxel
{
	class Application 
	{
	public:
		Application();
		~Application();

		static Application& get() { return *s_instance; }

		void add_layer(Layer* layer);

		void run() const;

		const VulkanContext& get_context() const { return m_context; }
		GameWindow& get_window() const { return *m_window; }

	private:
		static Application* s_instance;

		GameWindow* m_window;
		VulkanContext m_context;
		LayerStack m_LayerStack;
	};
}
