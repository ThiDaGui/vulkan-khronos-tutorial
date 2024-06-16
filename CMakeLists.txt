cmake_minimum_required(VERSION 3.29)
project(vulkan_khronos_tutorial)

set(CMAKE_CXX_STANDARD 17)

find_package(glfw3 REQUIRED)
find_package(Vulkan REQUIRED)


# vulkanTest executable

add_executable(vulkanTest vulkan_test.cpp)

target_compile_options(vulkanTest PRIVATE "-O2")

target_link_libraries(vulkanTest glfw Vulkan::Vulkan)

# drawTriangle executable

add_executable(drawTriangle main.cpp)

target_compile_options(drawTriangle PRIVATE "-O2")

target_link_libraries(drawTriangle glfw Vulkan::Vulkan)


add_executable(CreateDebugUtilsMessenger CreateDebugUtilsMessenger.cpp)

target_compile_options(CreateDebugUtilsMessenger PRIVATE "$<$<CONFIG:RELEASE>:-O2;-DNDEBUG>")

target_link_libraries(CreateDebugUtilsMessenger glfw Vulkan::Vulkan)