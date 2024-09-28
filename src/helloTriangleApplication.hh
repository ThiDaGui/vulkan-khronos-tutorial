#pragma once

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>

class HelloTriangleApplication {
private:
    GLFWwindow *window_;
    vk::Instance instance_;
    vk::DebugUtilsMessengerEXT debugMessenger_;

public:
    HelloTriangleApplication();

    void run();

private:
    void initWindow();

    void initVulkan();

    void createInstance();

    void setupDebugMessenger();
};
