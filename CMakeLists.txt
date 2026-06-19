cmake_minimum_required(VERSION 3.30)
project(vulkan_khronos_tutorial LANGUAGES CXX)

find_package(glfw3 CONFIG REQUIRED)
find_package(Vulkan REQUIRED)
find_package(slang CONFIG REQUIRED)
find_package(fastgltf CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(Stb REQUIRED)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)

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
            COMMAND slang::slangc -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name ${ENTRY_POINTS} -o ${SHADER_BINARY_DIR}/${TARGET_OUTPUT_NAME} -- ${SHADER_SOURCES}
            DEPENDS ${SHADER_SOURCES}
            COMMENT "Compiling slang shader ${TARGET}"
            VERBATIM
    )
    add_custom_target(${TARGET} DEPENDS ${SHADER_BINARY_DIR}/${TARGET_OUTPUT_NAME})
endfunction()

configure_file(config.hh.in config.hh @ONLY)

add_slang_shader(triangle_slang SOURCES ${SHADER_SOURCE_DIR}/triangle.slang)

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
        src/types/mem_allocator.cc
        src/types/descriptor_allocator.cc

        src/utils/pipeline_builder.cc
        src/utils/descriptor_set_layout_builder.cc
        src/types/buffer.cc
        src/types/Primitive.cc
)

target_include_directories(drawTriangleCpp PRIVATE
        ${Stb_INCLUDE_DIR}
)

target_sources(drawTriangleCpp
    PRIVATE
        FILE_SET internal_headers
        TYPE HEADERS
        BASE_DIRS
            "${CMAKE_BINARY_DIR}"
            external
            src
        FILES
            external/tiny_obj_loader/tiny_obj_loader.h

            src/engine/vk_engine.hh
            src/engine/required_queue_family_indices.hh
            src/engine/swapchain.hh
            src/engine/window_system.hh
            src/engine/core.hh

            src/types/image.hh
            src/types/pipeline.hh
            src/types/mem_allocator.hh
            src/types/descriptor_allocator.hh
            src/types/buffer.hh
            src/types/typed_buffer.hh
            src/types/Primitive.hh

            src/utils/types.hh
            src/utils/pipeline_builder.hh
            src/utils/descriptor_set_layout_builder.hh
            src/utils/required_features_checker.hh

            "${CMAKE_BINARY_DIR}/config.hh"
)

set_target_properties(drawTriangleCpp PROPERTIES
        CXX_STANDARD 20
)

target_compile_definitions(drawTriangleCpp PUBLIC
        VULKAN_HPP_NO_STRUCT_CONSTRUCTORS=1
)
target_link_libraries(drawTriangleCpp PRIVATE
        Vulkan::Vulkan
        GPUOpen::VulkanMemoryAllocator
        glfw
        fastgltf::fastgltf
        imgui::imgui
)

add_dependencies(drawTriangleCpp triangle_slang)