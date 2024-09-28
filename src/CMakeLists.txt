add_executable(drawTriangle2 main.cpp helloTriangleApplication.cpp helloTriangleApplication.hh vulkanUtils.cpp vulkanUtils.hh)

target_link_libraries(drawTriangle2 glfw Vulkan::Vulkan)
