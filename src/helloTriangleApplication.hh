#pragma once

import vulkan_hpp;

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vulkanUtils.hh"

constexpr uint32_t WIDTH = 800;
constexpr uint32_t HEIGHT = 600;

class HelloTriangleApplicationCpp {
private:
    GLFWwindow *window_;

    vk::raii::Context context_;

    vk::raii::Instance instance_ = nullptr;

    vk::raii::DebugUtilsMessengerEXT debug_messenger_ = nullptr;

    vk::raii::SurfaceKHR surface_ = nullptr;

    vk::raii::PhysicalDevice physical_device_ = nullptr;
    RequiredQueueFamilyIndices queue_family_indices_;

    vk::raii::Device device_ = nullptr;

    vk::raii::Queue graphic_queue = nullptr;
    vk::raii::Queue present_queue = nullptr;

    vk::raii::SwapchainKHR swapchain_ = nullptr;
    std::vector<vk::Image> swapchain_images_;
    vk::Format swapchain_image_format_ = vk::Format::eUndefined;
    vk::Extent2D swapchain_extent_;


public:
    explicit HelloTriangleApplicationCpp(GLFWwindow *window);

    ~HelloTriangleApplicationCpp();

    void run() const;

private:
    void initWindow();

    void initVulkan();

    void cleanup();

    void createInstance();

    void createSurface();

    void pickPhysicalDevice();

    void createLogicalDevice();

    void createSwapChain();

    void setupDebugMessenger();

    //-------------------------

    [[nodiscard]] vk::Extent2D chooseExtent2D(const vk::SurfaceCapabilitiesKHR& surface_capabilities) const;
};
