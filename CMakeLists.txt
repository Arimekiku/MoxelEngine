cmake_minimum_required(VERSION 3.12)

project(SDLarria)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(PROJECT_OUTPUT_DIR "build")
set(PROJECT_SOURCE_DIR "src")
set(RESOURCE_DIR "Resources/")

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

# three party
add_subdirectory("vendor" EXCLUDE_FROM_ALL)

################################# Executable ####################################

add_executable(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES})

target_compile_definitions("${CMAKE_PROJECT_NAME}" 
    PUBLIC
        RESOURCES_PATH="${RESOURCE_DIR}"
        $<$<CONFIG:Debug>:DEBUG>
        $<$<CONFIG:RelWithDebInfo>:RELEASE>
        $<$<CONFIG:Release>:RELEASE>
        $<$<CONFIG:MinSizeRel>:RELEASE>)

target_include_directories(${PROJECT_NAME} PRIVATE "${PROJECT_SOURCE_DIR}")
target_link_libraries(${PROJECT_NAME} PUBLIC libs PRIVATE ${CMAKE_DL_LIBS})

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

find_program(GLSL_VALIDATOR glslangValidator HINTS /usr/bin /usr/local/bin $ENV{VULKAN_SDK}/Bin/ $ENV{VULKAN_SDK}/Bin32/)
file(GLOB_RECURSE GLSL_SOURCE_FILES
    "${RESOURCE_DIR}**.frag"
    "${RESOURCE_DIR}**.vert"
    "${RESOURCE_DIR}**.comp"
    )

# compile shaders
foreach(GLSL ${GLSL_SOURCE_FILES})
    get_filename_component(FILE_NAME ${GLSL} NAME)
    set(SPIRV "${PROJECT_BINARY_DIR}/Resources/${FILE_NAME}.spv")
    add_custom_command(
            OUTPUT ${SPIRV}
            COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_BINARY_DIR}/Resources/"
            COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
            DEPENDS ${GLSL})
    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

add_custom_target(Shaders DEPENDS ${SPIRV_BINARY_FILES})
add_dependencies(SDLarria Shaders)

add_custom_command(TARGET SDLarria POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:SDLarria>/Resources/"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_BINARY_DIR}/Resources"
        "$<TARGET_FILE_DIR:SDLarria>/Resources"
)

# define resources in binaries
file(COPY ${RESOURCE_DIR} DESTINATION "${CMAKE_BINARY_DIR}/Resources/")