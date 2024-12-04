cmake_minimum_required(VERSION 3.12)

project(SDLarria)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(PROJECT_OUTPUT_DIR "./build")
set(PROJECT_SOURCE_DIR "./src")
set(RESOURCE_DIR "./Resources/")

# hot reload for MSVC compilers if supported
if (MSVC)
    set(CMAKE_CXX_FLAGS "/Zi /EHsc")

    if (POLICY CMP0141)
      cmake_policy(SET CMP0141 NEW)
      set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
    endif()
endif()

# define headers
file(GLOB_RECURSE HEADER_FILES "${PROJECT_SOURCE_DIR}/**.h")

# define source
file(GLOB_RECURSE SOURCE_FILES "${PROJECT_SOURCE_DIR}/**.cpp")

# define resources in binaries
file(COPY ${RESOURCE_DIR} DESTINATION "${CMAKE_BINARY_DIR}/Resources/")

# vulkan
find_package(Vulkan REQUIRED)

# vk-bootstrap
set(VK_BOOT_DIR "vendor/vk-bootstrap")
add_subdirectory(${VK_BOOT_DIR} EXCLUDE_FROM_ALL)

# SDL
set(SDL_DIR "vendor/SDL")
set(SDL_STATIC ON)
add_subdirectory(${SDL_DIR} EXCLUDE_FROM_ALL)

# spdlog
set(SPDLOG_DIR "vendor/spdlog")
add_subdirectory(${SPDLOG_DIR} EXCLUDE_FROM_ALL)

################################# Executable ####################################

add_executable(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES})

target_compile_definitions("${CMAKE_PROJECT_NAME}" 
    PUBLIC
        RESOURCES_PATH="${RESOURCE_DIR}"
        $<$<CONFIG:Debug>:DEBUG>
        $<$<CONFIG:RelWithDebInfo>:RELEASE>
        $<$<CONFIG:Release>:RELEASE>
        $<$<CONFIG:MinSizeRel>:RELEASE>)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        "${Vulkan_INCLUDE_DIRS}"
    PRIVATE
        "${PROJECT_SOURCE_DIR}")

target_link_libraries(${PROJECT_NAME} PUBLIC SDL3::SDL3-static vk-bootstrap::vk-bootstrap ${Vulkan_LIBRARIES} spdlog PRIVATE ${CMAKE_DL_LIBS})

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})