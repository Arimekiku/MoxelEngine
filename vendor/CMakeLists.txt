cmake_minimum_required(VERSION 3.12)

project(libs)

# vulkan 
find_package(Vulkan REQUIRED)

# vk-bootstrap
set(VK_BOOT_DIR "vk-bootstrap")
add_subdirectory(${VK_BOOT_DIR} EXCLUDE_FROM_ALL)

# SDL
set(SDL_DIR "SDL")
add_subdirectory(${SDL_DIR} EXCLUDE_FROM_ALL)

# imgui
set(IMGUI_DIR "ImGui")
file(GLOB IMGUI_SOURCES "${IMGUI_DIR}/imgui.h" "${IMGUI_DIR}/backends/imgui_impl_sdl2.h" "${IMGUI_DIR}/backends/imgui_impl_vulkan.h")

################################# Library ####################################

add_library(${PROJECT_NAME} ${IMGUI_SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(${PROJECT_NAME} 
	PUBLIC 
		"${IMGUI_DIR}"
		"${VK_BOOT_DIR}/src"
		"${Vulkan_INCLUDE_DIRS}")

target_link_libraries(${PROJECT_NAME} PUBLIC SDL3::SDL3 vk-bootstrap::vk-bootstrap ${Vulkan_LIBRARIES})