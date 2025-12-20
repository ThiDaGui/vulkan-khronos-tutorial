//
// Created by damiendidier on 18/11/2025.
//

#pragma once

#include <cstdint>
#include <span>
#include <vector>

#include "required_queue_family_indices.hh"

import vulkan_hpp;

struct GLFWwindow;

namespace vk_tutorial
{
struct WindowSystem {
    GLFWwindow *window{nullptr};

    WindowSystem() = default;
    ~WindowSystem();

    WindowSystem(const WindowSystem &other) = delete;
    WindowSystem &operator=(const WindowSystem &other) = delete;
    WindowSystem(WindowSystem &&other) = delete;
    WindowSystem &operator=(WindowSystem &&other) = delete;

    void init(std::uint32_t width, uint32_t height, const char *name);

    static std::vector<const char *> getRequiredExtensions();

    [[nodiscard]] vk::raii::SurfaceKHR createSurface(const vk::raii::Instance &instance) const;

    [[nodiscard]] vk::Extent2D getExtent() const;

    [[nodiscard]] bool shouldClose() const;
};

class VkEngine
{
public:
    bool is_initialized{false};

    vk::Extent2D window_extent{800, 600};

private:
    WindowSystem window_system_{};

    vk::raii::Context context_{};

    vk::raii::Instance instance_{nullptr};
    vk::raii::PhysicalDevice physical_device_{nullptr};
    vk::raii::Device device_{nullptr};

    vk::raii::DebugUtilsMessengerEXT debug_messenger_{nullptr};

    vk::raii::SurfaceKHR surface_{nullptr};

    RequiredQueueFamilyIndices queue_family_indices_{};
    vk::raii::Queue graphics_queue_{nullptr};
    vk::raii::Queue present_queue_{nullptr};

    vk::raii::SwapchainKHR swapchain_{nullptr};
    uint32_t swapchain_min_image_count_{};
    vk::Extent2D swapchain_extent_{};
    vk::Format swapchain_image_format_{};
    std::vector<vk::Image> swapchain_images_{};
    std::vector<vk::raii::ImageView> swapchain_image_views_{};

public:
    VkEngine();

    ~VkEngine();

    void run();

private:
    void initVulkan();

    void createInstance(const std::vector<const char *> &instance_extensions, const std::vector<const char *> &instance_layers);

    void createDebugMessenger();

    void pickPhysicalDevice(std::span<const char * const> device_extensions);

    void createDevice();

    static uint32_t chooseMinImageCount(const vk::SurfaceCapabilitiesKHR &surface_capabilities) ;
    void createSwapchain();

    [[nodiscard]] vk::Extent2D chooseExtent2D(const vk::SurfaceCapabilitiesKHR &surface_capabilities) const;
    static vk::SurfaceFormatKHR chooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &surface_formats) ;
    static vk::PresentModeKHR choosePresentMode(const std::vector<vk::PresentModeKHR> &present_modes);

    static uint32_t gradePhysicalDevice(const vk::raii::PhysicalDevice &physical_device, const vk::raii::SurfaceKHR &surface, std::span<const char * const> required_extensions);

    void draw();

    static constexpr std::array required_device_extensions = {
        vk::KHRSwapchainExtensionName,
        vk::KHRSpirv14ExtensionName,
        vk::KHRSynchronization2ExtensionName,
        vk::KHRCreateRenderpass2ExtensionName,
    };

};
}