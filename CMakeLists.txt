cmake_minimum_required(VERSION 3.28)
project(vulkan_khronos_tutorial CXX)

set(CMAKE_CXX_STANDARD 17)

find_package(glfw3 REQUIRED)
find_package(Vulkan REQUIRED COMPONENTS glslc)

add_compile_options(
        "$<$<CONFIG:Debug>:-Wall;-Wextra;-O0;-g;-fsanitize=address;-fsanitize=undefined>") #-Werror;

	add_link_options("$<$<CONFIG:Debug>:-fsanitize=address;-fsanitize=undefined>")

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

# vulkanTest executable

# add_executable(vulkanTest vulkan_test.cpp)
 
# target_link_libraries(vulkanTest glfw Vulkan::Vulkan)


# drawTriangle executable

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

target_include_directories(drawTriangle PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(drawTriangle glfw Vulkan::Vulkan)


#add_executable(CreateDebugUtilsMessenger CreateDebugUtilsMessenger.cpp)
#
#target_link_libraries(CreateDebugUtilsMessenger glfw Vulkan::Vulkan)
