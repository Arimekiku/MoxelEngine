#include "gui_layer.h"
#include "engine/renderer/vulkan.h"
#include "engine/application.h"
#include "engine/renderer/vulkan_renderer.h"

#include <backends/imgui_impl_vulkan.cpp>
#include <backends/imgui_impl_sdl3.cpp>

namespace Moxel
{
    void GuiLayer::attach()
    {
		const auto& vulkan = Application::get().get_context();
        const VkDescriptorPoolSize poolSizes[] =
        { 
            { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 } 
        };

        auto poolCreateInfo = VkDescriptorPoolCreateInfo();
        poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        poolCreateInfo.maxSets = 1000;
        poolCreateInfo.poolSizeCount = static_cast<uint32_t>(std::size(poolSizes));
        poolCreateInfo.pPoolSizes = poolSizes;

        const auto result = vkCreateDescriptorPool(vulkan.get_logical_device(), &poolCreateInfo, nullptr, &m_imguiPool);
        VULKAN_CHECK(result);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

        const auto window = Application::get().get_window().get_native_window();
        ImGui_ImplSDL3_InitForVulkan(window);

        auto& swapchain = VulkanRenderer::get_swapchain();
		auto createInfo = VkPipelineRenderingCreateInfo();
		createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
		createInfo.colorAttachmentCount = 1;
		createInfo.pColorAttachmentFormats = &swapchain.get_image_format();

        auto initInfo = ImGui_ImplVulkan_InitInfo();
        initInfo.Instance = vulkan.get_instance();
        initInfo.PhysicalDevice = vulkan.get_physical_device();
        initInfo.Device = vulkan.get_logical_device();
        initInfo.Queue = vulkan.get_render_queue();
		initInfo.DescriptorPool = m_imguiPool;
        initInfo.MinImageCount = VulkanRenderer::get_specifications().FRAMES_IN_FLIGHT;
		initInfo.ImageCount = VulkanRenderer::get_specifications().FRAMES_IN_FLIGHT;
        initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        initInfo.CheckVkResultFn = VulkanUtils::vulkan_check;
        initInfo.UseDynamicRendering = true;
        initInfo.PipelineRenderingCreateInfo = createInfo;

        ImGui_ImplVulkan_Init(&initInfo);
        ImGui_ImplVulkan_CreateFontsTexture();

        ImGui::StyleColorsDark();
    }

    void GuiLayer::detach()
    {
        const auto device = Application::get().get_context().get_logical_device();

        vkDeviceWaitIdle(device);

        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplSDL3_Shutdown();

        vkDestroyDescriptorPool(device, m_imguiPool, nullptr);
        ImGui::DestroyContext();
    }

    void GuiLayer::process_events(const SDL_Event& event)
    {
        ImGui_ImplSDL3_ProcessEvent(&event);
    }

    void GuiLayer::begin()
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
    }

    void GuiLayer::end()
    {
        ImGui::Render();
    }
}
