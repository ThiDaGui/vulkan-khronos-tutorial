cmake_minimum_required(VERSION 3.29)
project(vulkan_khronos_tutorial CXX)

set(CMAKE_CXX_STANDARD 17)

find_package(glfw3 REQUIRED)
find_package(Vulkan REQUIRED)

add_compile_options(
        "$<$<CONFIG:Debug>:-Wall;-Wextra;-Werror;-O1;-g;-fsanitize=address>")

add_link_options("$<$<CONFIG:Debug>:-fsanitize=address>")

# vulkanTest executable

add_executable(vulkanTest vulkan_test.cpp)

target_link_libraries(vulkanTest glfw Vulkan::Vulkan)


# drawTriangle executable

add_executable(drawTriangle main.cpp)

target_link_libraries(drawTriangle glfw Vulkan::Vulkan)


add_executable(CreateDebugUtilsMessenger CreateDebugUtilsMessenger.cpp)

target_link_libraries(CreateDebugUtilsMessenger glfw Vulkan::Vulkan)