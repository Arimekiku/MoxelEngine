#include "gui_layer.h"
#include "engine/vulkan/vulkan_engine.h"
#include "engine/application.h"

#include <backends/imgui_impl_vulkan.cpp>
#include <backends/imgui_impl_sdl3.cpp> 
#include <engine/vulkan/vulkan.h>

namespace SDLarria
{
    void GuiLayer::Attach()
    {
        auto vulkan = VulkanEngine::Get().GetInstance();

;       VkDescriptorPoolSize pool_sizes[] = { 
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

        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = 1000;
        pool_info.poolSizeCount = (uint32_t)std::size(pool_sizes);
        pool_info.pPoolSizes = pool_sizes;

        VkDescriptorPool imguiPool;
        auto result = vkCreateDescriptorPool(vulkan.GetLogicalDevice(), &pool_info, nullptr, &imguiPool);
        VULKAN_CHECK(result);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

        auto window = Application::Get().GetWindow().GetNativeWindow();
        ImGui_ImplSDL3_InitForVulkan(window);

        auto init_info = ImGui_ImplVulkan_InitInfo();
        init_info.Instance = vulkan.GetInstance();
        init_info.PhysicalDevice = vulkan.GetPhysicalDevice();
        init_info.Device = vulkan.GetLogicalDevice();
        init_info.QueueFamily = vulkan.GetQueueFamilyIndex();
        init_info.Queue = vulkan.GetRenderQueue();
        init_info.DescriptorPool = imguiPool;
        init_info.Subpass = 0;
        init_info.MinImageCount = 3;
        init_info.ImageCount = 3;
        init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        init_info.CheckVkResultFn = VulkanUtils::VulkanCheck;

        auto swapchain = VulkanEngine::Get().GetSwapchain();
        auto createInfo = VkPipelineRenderingCreateInfo();
        createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
        createInfo.colorAttachmentCount = 1;
        createInfo.pColorAttachmentFormats = &swapchain.GetImageFormat();

        init_info.UseDynamicRendering = true;
        init_info.PipelineRenderingCreateInfo = createInfo;

        ImGui_ImplVulkan_Init(&init_info);
        ImGui_ImplVulkan_CreateFontsTexture();

        ImGui::StyleColorsDark();
    }

    void GuiLayer::Detach()
    {
        vkDeviceWaitIdle(VulkanEngine::Get().GetInstance().GetLogicalDevice());

        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();
    }

    void GuiLayer::Begin()
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
        
        ImGui::ShowDemoWindow();
    }

    void GuiLayer::End()
    {
        ImGui::Render();
    }
}