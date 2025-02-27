#pragma once

#include "core/core.h"
#include "renderer/vulkan_allocator.h"
#include "renderer/vulkan_context.h"
#include "ui/gui_layer.h"

namespace Moxel
{
	class Application 
	{
	public:
		Application();
		~Application();

		static Application& get() { return *s_instance; }

		void run() const;

		void add_layer(Layer* layer);

		VulkanAllocator& get_allocator() { return m_allocator; }
		const VulkanContext& get_context() const { return m_context; }
		GameWindow& get_window() const { return *m_window; }

	private:
		static Application* s_instance;

		GameWindow* m_window;
		VulkanAllocator m_allocator;
		VulkanContext m_context;

		LayerStack m_layerStack;
		GuiLayer* m_guiLayer;
	};
}
