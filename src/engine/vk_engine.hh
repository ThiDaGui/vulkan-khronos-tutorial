//
// Created by damiendidier on 18/11/2025.
//

#pragma once

#include <vulkan/vulkan_raii.hpp>

#include "core.hh"
#include "required_queue_family_indices.hh"
#include "swapchain.hh"
#include "window_system.hh"
#include "types/image.hh"
#include "types/pipeline.hh"

namespace vk_tutorial
{
class VkEngine
{
    static constexpr std::array REQUIRED_DEVICE_EXTENSIONS = {
        vk::KHRSwapchainExtensionName,
        vk::KHRSpirv14ExtensionName,
    };

    static constexpr uint32_t FRAME_OVERLAP = 2;

public:
    bool is_initialized{false};

    vk::Extent2D window_extent{800, 600};

private:
    WindowSystem window_system_{};

    Core core_{};

    Swapchain swapchain_{};

    uint32_t fence_index_{0};
    uint32_t semaphore_index_{0};
    std::vector<vk::raii::Fence> in_flight_fences_{};

    vk_types::Image color_render_target_{};

	vk_types::Pipeline pipeline{nullptr, nullptr};

public:
    VkEngine();

    ~VkEngine();

    void run();

private:
    void draw();
};
}
