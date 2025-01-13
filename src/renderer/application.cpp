#include "application.h"
#include "ui/gui_layer.h"
#include "vulkan/vulkan_renderer.h"

namespace Moxel
{
	Application* Application::s_instance;

	Application::Application()
	{
		s_instance = this;

		Log::initialize();

		constexpr auto initialSize = VkExtent2D(1600, 900);
		m_window = new GameWindow(initialSize.width, initialSize.height);
		m_context.initialize(m_window->get_native_window());
		m_allocator.initialize();

		VulkanRenderer::initialize(initialSize);

		m_LayerStack = LayerStack();
	}

	Application::~Application()
	{
		vkDeviceWaitIdle(m_context.get_logical_device());

		m_LayerStack.clear();

		VulkanRenderer::shutdown();

		m_allocator.destroy();
		m_context.destroy();
	}

	void Application::run() const
	{
		m_window->update(m_LayerStack);
	}

	void Application::add_layer(Layer* layer)
	{
		m_LayerStack.push(layer);
		layer->attach();
	}
}
