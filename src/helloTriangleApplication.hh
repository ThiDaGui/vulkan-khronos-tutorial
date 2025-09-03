#pragma once

import vulkan_hpp;

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vulkanUtils.hh"

constexpr uint32_t WIDTH = 800;
constexpr uint32_t HEIGHT = 600;

constexpr uint32_t MAX_FRAME_IN_FLIGHT = 2;

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
    std::vector<vk::raii::ImageView> swapchain_image_views_;
    bool framebufferResized = false;

    vk::raii::PipelineLayout pipeline_layout_ = nullptr;
    vk::raii::Pipeline graphic_pipeline_ = nullptr;

    vk::raii::CommandPool command_pool_ = nullptr;
    std::vector<vk::raii::CommandBuffer> command_buffer_;

    std::vector<vk::raii::Semaphore> present_complete_semaphores_;
    std::vector<vk::raii::Semaphore> render_finished_semaphores_;
    std::vector<vk::raii::Fence> in_flight_fences_;

    uint32_t current_frame_ = 0;
    uint32_t semaphore_index_ = 0;


public:
    HelloTriangleApplicationCpp();

    ~HelloTriangleApplicationCpp();

    void run();

private:
    void initWindow();

    void initVulkan();

    void cleanup();

    void createInstance();

    void createSurface();

    void pickPhysicalDevice();

    void createLogicalDevice();

    void createSwapChain();

    void createSwapchainImageView();

    void createGraphicPipeline();

    void createCommandPool();

    void createCommandBuffers();

    void recordCommandBuffers() const;

    void recordCommandBuffer(uint32_t image_index) const;

    void createSyncObject();

    //-------------------------

    void cleanupSwapChain();

    void recreateSwapChain();

    //-------------------------

    void drawFrame();

    //-------------------------

    void setupDebugMessenger();

    //-------------------------

    [[nodiscard]] vk::Extent2D chooseExtent2D(const vk::SurfaceCapabilitiesKHR& surface_capabilities) const;

    [[nodiscard]] vk::raii::ShaderModule createShaderModule(const std::filesystem::path& shader_path) const;

    static void framebufferResizeCallback(GLFWwindow *window, int width, int height);
};
