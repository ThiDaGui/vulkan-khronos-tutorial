cmake_minimum_required(VERSION 3.28)
project(vulkan_khronos_tutorial CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

find_package(glfw3 REQUIRED)
find_package(Vulkan REQUIRED COMPONENTS glslc)

set(SHADER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
set(SHADER_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders)

file(MAKE_DIRECTORY ${SHADER_BINARY_DIR})

function(add_spirv_shader INPUT_SHADER OUTPUT_SHADER)
    add_custom_command(
            OUTPUT ${OUTPUT_SHADER}
            COMMAND Vulkan::glslc ${INPUT_SHADER} -o ${OUTPUT_SHADER}
            DEPENDS ${INPUT_SHADER}
    )
endfunction()

configure_file(config.hh.in config.hh @ONLY)

add_spirv_shader(
        ${SHADER_SOURCE_DIR}/triangle.vert ${SHADER_BINARY_DIR}/triangle_vert.spv)
add_spirv_shader(
        ${SHADER_SOURCE_DIR}/triangle.frag ${SHADER_BINARY_DIR}/triangle_frag.spv)

add_custom_target(shaders
        DEPENDS
        ${SHADER_BINARY_DIR}/triangle_frag.spv
        ${SHADER_BINARY_DIR}/triangle_vert.spv
)

add_executable(drawTriangle main.cpp)

add_dependencies(drawTriangle shaders)

target_include_directories(drawTriangle PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
        ${Vulkan_INCLUDE_DIR}
)

target_compile_options(drawTriangle PRIVATE
        "$<$<CONFIG:Debug>:-Wall;-Wextra;-O0;-g>") #-Werror;

set_target_properties(drawTriangle PROPERTIES
        CXX_STANDARD 17
)

target_link_libraries(drawTriangle glfw Vulkan::Vulkan)

add_subdirectory(src)