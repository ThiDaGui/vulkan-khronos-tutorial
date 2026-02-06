#pragma once

#include <vulkan/vulkan_raii.hpp>

namespace vk_tutorial
{
struct Core;
struct WindowSystem;
struct RequiredQueueFamilyIndices;

struct PerFrame
{
    vk::raii::ImageView image_view{nullptr};
    vk::raii::CommandBuffer command_buffer{nullptr};
    vk::raii::Semaphore is_presentable_semaphore{nullptr};
};

struct Swapchain
{
    void init(const Core& core,
              const WindowSystem& window_system);

    [[nodiscard]] std::pair<vk::Result, uint32_t> Acquire(uint32_t semaphore_index) const;
    [[nodiscard]] vk::Result Present(const vk::raii::Queue& present_queue, uint32_t image_index) const;

    void recreate(const Core& core, const WindowSystem& window_system);

    vk::raii::SwapchainKHR vk_swapchain{nullptr};
    uint32_t min_image_count{};
    uint32_t image_count{};
    vk::Extent2D extent{};
    vk::Format image_format{};

    std::vector<vk::Image> images{};

    std::vector<vk::raii::Semaphore> frame_acquired_semaphores{};

    std::vector<PerFrame> frames_data{};

private:
    static vk::Extent2D chooseExtent2D(const WindowSystem& window_system,
                                       const vk::SurfaceCapabilitiesKHR& surface_capabilities);
    static uint32_t chooseMinImageCount(const vk::SurfaceCapabilitiesKHR& surface_capabilities);
    static vk::SurfaceFormatKHR chooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& surface_formats);
    static vk::PresentModeKHR choosePresentMode(const std::vector<vk::PresentModeKHR>& present_modes);
};
}
