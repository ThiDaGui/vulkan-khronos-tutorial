cmake_minimum_required(VERSION 3.30)
project(vulkan_khronos_tutorial LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

find_package(glfw3 REQUIRED)
find_package(Vulkan REQUIRED COMPONENTS slangc)

set(DATA_DIR ${CMAKE_SOURCE_DIR}/data)
set(SHADER_SOURCE_DIR ${CMAKE_SOURCE_DIR}/shaders)
set(SHADER_BINARY_DIR ${CMAKE_BINARY_DIR}/shaders)

file(MAKE_DIRECTORY ${SHADER_BINARY_DIR})

function(add_slang_shader TARGET)
    cmake_parse_arguments("SHADER" "" "" "SOURCES" ${ARGN})
    set(ENTRY_POINTS -entry vertMain -entry fragMain)
    string(JOIN "." TARGET_OUTPUT_NAME "${TARGET}" "spv")
    add_custom_command(
            OUTPUT ${SHADER_BINARY_DIR}/${TARGET_OUTPUT_NAME}
            COMMAND Vulkan::slangc -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name ${ENTRY_POINTS} -o ${SHADER_BINARY_DIR}/${TARGET_OUTPUT_NAME} -- ${SHADER_SOURCES}
            DEPENDS ${SHADER_SOURCES}
            COMMENT "Compiling slang shader ${TARGET}"
            VERBATIM
    )
    add_custom_target(${TARGET} DEPENDS ${SHADER_BINARY_DIR}/${TARGET_OUTPUT_NAME})
endfunction()

configure_file(config.hh.in config.hh @ONLY)

add_slang_shader(triangle_slang SOURCES ${SHADER_SOURCE_DIR}/triangle.slang)

add_library(Imgui
        external/imgui/imgui.cpp
        external/imgui/imgui_demo.cpp
        external/imgui/imgui_draw.cpp
        external/imgui/imgui_tables.cpp
        external/imgui/imgui_widgets.cpp

        external/imgui/backends/imgui_impl_glfw.cpp
        external/imgui/backends/imgui_impl_vulkan.cpp
)

target_sources(Imgui
        PUBLIC
        FILE_SET public_headers
        TYPE HEADERS
        BASE_DIRS external/imgui
        FILES
            external/imgui/imgui.h
            external/imgui/backends/imgui_impl_vulkan.h
            external/imgui/backends/imgui_impl_glfw.h
)

target_include_directories(Imgui
        PRIVATE "${Vulkan_INCLUDE_DIR}"
)

add_executable(drawTriangleCpp
        src/main.cpp

        external/stb_image/stb_image.cc
        external/tiny_obj_loader/tiny_obj_loader.cc
        external/vk_mem_alloc/vma_usage.cc

        src/engine/vk_engine.cc
        src/engine/required_queue_family_indices.cc
        src/engine/swapchain.cc
        src/engine/window_system.cc
        src/engine/core.cc

        src/types/image.cc
        src/types/pipeline.cc
)

target_sources(drawTriangleCpp
    PRIVATE
        FILE_SET internal_headers
        TYPE HEADERS
        BASE_DIRS
            "${Vulkan_INCLUDE_DIR}"
            "${CMAKE_BINARY_DIR}"
            external
            src
        FILES
            "external/stb_image/stb_image.h"
            "external/tiny_obj_loader/tiny_obj_loader.h"
            "external/vk_mem_alloc/vk_mem_alloc.h"
            "external/vk_mem_alloc/vma_usage.hh"

            "src/engine/vk_engine.hh"
            "src/engine/required_queue_family_indices.hh"
            "src/engine/swapchain.hh"
            "src/engine/window_system.hh"
            "src/engine/core.hh"

            "src/types/image.hh"
            "src/types/pipeline.hh"

            "${CMAKE_BINARY_DIR}/config.hh"
)

set_target_properties(drawTriangleCpp PROPERTIES
        CXX_STANDARD 20
)

target_compile_definitions(drawTriangleCpp PUBLIC
        VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
        VULKAN_HPP_NO_STRUCT_CONSTRUCTORS=1
)
target_link_libraries(drawTriangleCpp PRIVATE
        Vulkan::Vulkan
        glfw
        Imgui
)

add_dependencies(drawTriangleCpp triangle_slang)