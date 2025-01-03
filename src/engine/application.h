#pragma once

#include "core/core.h"
#include "vulkan/vulkan_context.h"

namespace SDLarria 
{
	class Application 
	{
	public:
		Application();
		~Application();

		static Application& Get() { return *s_Instance; }

		void AddLayer(Layer* layer);

		void Run() const;

		const VulkanContext& GetContext() const { return m_Context; }
		GameWindow& GetWindow() const { return *m_Window; }

	private:
		static Application* s_Instance;

		GameWindow* m_Window;
		VulkanContext m_Context;
		LayerStack m_LayerStack;
	};
}
