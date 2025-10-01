cmake_minimum_required(VERSION 3.28)
project(vulkan_khronos_tutorial CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

find_package(glfw3 REQUIRED)
find_package(Vulkan REQUIRED COMPONENTS slangc)

set(SHADER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
set(SHADER_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders)
set(DATA_PATH ${CMAKE_SOURCE_DIR}/data)

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

add_subdirectory(src)