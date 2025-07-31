#pragma once

import vulkan_hpp;

#include <GLFW/glfw3.h>

class HelloTriangleApplication {
private:
    GLFWwindow *window_;

    vk::raii::Context context_;

    vk::raii::Instance instance_ = nullptr;

public:
    HelloTriangleApplication();
    ~HelloTriangleApplication();

    void run();

private:
    void initWindow();

    void initVulkan();

    void cleanup();

    void createInstance();

    void setupDebugMessenger();
};
