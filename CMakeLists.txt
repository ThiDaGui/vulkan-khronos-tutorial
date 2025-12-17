cmake_minimum_required(VERSION 3.28)
project(vulkan_khronos_tutorial CXX)

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

add_library(VulkanCppModule)
add_library(Vulkan::cppm ALIAS VulkanCppModule)

target_compile_definitions(VulkanCppModule PUBLIC
        VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
        VULKAN_HPP_NO_STRUCT_CONSTRUCTORS=1
)

target_include_directories(VulkanCppModule PRIVATE
        "${Vulkan_INCLUDE_DIR}"
)

target_link_libraries(VulkanCppModule PUBLIC
        Vulkan::Vulkan
)

set_target_properties(VulkanCppModule PROPERTIES
        CXX_STANDARD 20
)

target_sources(VulkanCppModule PUBLIC
        FILE_SET cxx_modules TYPE CXX_MODULES
        BASE_DIRS "${Vulkan_INCLUDE_DIR}"
        FILES "${Vulkan_INCLUDE_DIR}/vulkan/vulkan.cppm"
)

add_executable(drawTriangleCpp
        src/helloTriangleApplication.cpp
        src/vulkanUtils.cpp
        src/stb_image.cc
        src/vertex.cc
        src/main.cpp

        src/helloTriangleApplication.hh
        src/vulkanUtils.hh
        src/vertex.hh
        src/tiny_obj_loader.cc

        external/imgui/imgui.cpp
        external/imgui/imgui_demo.cpp
        external/imgui/imgui_draw.cpp
        external/imgui/imgui_tables.cpp
        external/imgui/imgui_widgets.cpp

        external/imgui/backends/imgui_impl_glfw.cpp
        external/imgui/backends/imgui_impl_vulkan.cpp
        src/engine/vk_engine.cc
        src/engine/vk_engine.hh
        src/engine/required_queue_family_indices.cc
        src/engine/required_queue_family_indices.hh
)

target_include_directories(drawTriangleCpp PRIVATE
        "${Vulkan_INCLUDE_DIR}"
        "${CMAKE_BINARY_DIR}"
        external
        external/imgui
)

set_target_properties(drawTriangleCpp PROPERTIES
        CXX_STANDARD 20
)

target_link_libraries(drawTriangleCpp PRIVATE
        Vulkan::cppm
        glfw
)

add_dependencies(drawTriangleCpp triangle_slang)