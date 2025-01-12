#include "gui_layer.h"
#include "renderer/vulkan/vulkan.h"
#include "renderer/application.h"
#include "renderer/vulkan/vulkan_renderer.h"

#include <backends/imgui_impl_vulkan.cpp>
#include <backends/imgui_impl_sdl3.cpp>

namespace Moxel
{
    static VkDescriptorPool s_Test;

    void GuiLayer::attach()
    {
		const auto& vulkan = Application::get().get_context();
        const VkDescriptorPoolSize pool_sizes[] =
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

        auto pool_info = VkDescriptorPoolCreateInfo();
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = 1000;
        pool_info.poolSizeCount = static_cast<uint32_t>(std::size(pool_sizes));
        pool_info.pPoolSizes = pool_sizes;

        const auto result = vkCreateDescriptorPool(vulkan.get_logical_device(), &pool_info, nullptr, &s_Test);
        VULKAN_CHECK(result);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

        const auto window = Application::get().get_window().get_native_window();
        ImGui_ImplSDL3_InitForVulkan(window);

        auto initInfo = ImGui_ImplVulkan_InitInfo();
        initInfo.Instance = vulkan.get_instance();
        initInfo.PhysicalDevice = vulkan.get_physical_device();
        initInfo.Device = vulkan.get_logical_device();
        initInfo.Queue = vulkan.get_render_queue();
        initInfo.DescriptorPool = s_Test;
        initInfo.MinImageCount = 3;
        initInfo.ImageCount = 3;
        initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        initInfo.CheckVkResultFn = VulkanUtils::vulkan_check;

        auto& swapchain = VulkanRenderer::get_swapchain();
        auto createInfo = VkPipelineRenderingCreateInfo();
        createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
        createInfo.colorAttachmentCount = 1;
        createInfo.pColorAttachmentFormats = &swapchain.get_image_format();

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

        vkDestroyDescriptorPool(device, s_Test, nullptr);
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
