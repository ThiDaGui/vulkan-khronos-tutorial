//
// Created by damiendidier on 18/11/2025.
//

#pragma once

#include <cstdint>
#include <vector>

import vulkan_hpp;

struct GLFWwindow;

namespace vk_tutorial {

struct WindowSystem {
    GLFWwindow *window{nullptr};

    WindowSystem() = default;
    ~WindowSystem();

    WindowSystem(const WindowSystem &other) = delete;
    WindowSystem &operator=(const WindowSystem &other) = delete;
    WindowSystem(WindowSystem &&other) = delete;
    WindowSystem &operator=(WindowSystem &&other) = delete;

    void init(std::uint32_t width, uint32_t height, const char *name);

    std::vector<const char *> getRequiredExtensions() ;

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

public:
    VkEngine();

    ~VkEngine();

    void run();

private:
    void initVulkan();

    void createInstance(const std::vector<const char *> &instance_extensions, const std::vector<const char *> &instance_layers);

    void createDebugMessenger();

    void pickPhysicalDevice(const std::vector<const char *> & device_extensions);

    static uint32_t gradePhysicalDevice(const vk::raii::PhysicalDevice &physical_device, const std::vector<const char *> &required_extensions);

    void draw();
};
}