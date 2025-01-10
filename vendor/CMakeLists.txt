cmake_minimum_required(VERSION 3.12)

project(libs)

# vulkan
find_package(Vulkan REQUIRED)

# vma 
set(VMA_DIR "VulkanMemoryAllocator")
add_subdirectory(${VMA_DIR} EXCLUDE_FROM_ALL)

# vk-bootstrap
set(VK_BOOT_DIR "vk-bootstrap")
add_subdirectory(${VK_BOOT_DIR} EXCLUDE_FROM_ALL)

# glm
set(GLM_DIR "glm")
add_subdirectory(${GLM_DIR} EXCLUDE_FROM_ALL)

# SDL
set(SDL_DIR "SDL")
set(SDL_STATIC ON)
add_subdirectory(${SDL_DIR} EXCLUDE_FROM_ALL)

# spdlog
set(SPDLOG_DIR "spdlog")
add_subdirectory(${SPDLOG_DIR} EXCLUDE_FROM_ALL)

# imgui
set(IMGUI_DIR "imgui")
file(GLOB IMGUI_SOURCES 
        "${IMGUI_DIR}/imgui.h"
        "${IMGUI_DIR}/imgui.cpp"
        "${IMGUI_DIR}/imgui_demo.cpp"
        "${IMGUI_DIR}/imgui_draw.cpp"
        "${IMGUI_DIR}/imgui_widgets.cpp"
        "${IMGUI_DIR}/imgui_tables.cpp"
        "${IMGUI_DIR}/imgui_impl_vulkan.cpp"
        "${IMGUI_DIR}/imgui_impl_sdl3.cpp"
    )

# perlin
set(PERLIN_DIR "PerlinNoise")
file(GLOB PERLIN_SOURCE "${PERLIN_DIR}/PerlinNoise.hpp")

################################# Library ####################################

add_library(${PROJECT_NAME} ${IMGUI_SOURCES} ${PERLIN_SOURCE})
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(${PROJECT_NAME}
	PUBLIC
		"${IMGUI_DIR}"
		"${PERLIN_DIR}"
		"${VK_BOOT_DIR}/src"
		"${Vulkan_INCLUDE_DIRS}"
    )

target_link_libraries(${PROJECT_NAME} PUBLIC ${Vulkan_LIBRARIES} GPUOpen::VulkanMemoryAllocator vk-bootstrap::vk-bootstrap spdlog SDL3::SDL3-static glm::glm)