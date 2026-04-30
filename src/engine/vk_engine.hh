#pragma once

#include <glm/mat4x4.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "core.hh"
#include "swapchain.hh"
#include "window_system.hh"
#include "types/buffer.hh"
#include "types/descriptor_allocator.hh"
#include "types/image.hh"
#include "types/pipeline.hh"

namespace vk_tutorial
{
struct VP
{
    glm::mat4 view_matrix;
    glm::mat4 projection_matrix;
};


class VkEngine final : NonCopyable
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

    uint32_t in_flight_index_{0};
    std::vector<vk::raii::Fence> in_flight_fences_{};

    std::vector<vk_types::Image> color_render_target_{};

    vk_types::DescriptorAllocator descriptor_allocator_{};
    std::array<vk::DescriptorSet, FRAME_OVERLAP> descriptor_set_{};
    vk::raii::DescriptorSetLayout descriptor_set_layout_{nullptr};

    std::array<vk_types::Buffer, FRAME_OVERLAP> view_proj_uniform_{};
    vk_types::Buffer mesh{};
    vk::DeviceAddress mesh_address;

    vk_types::Pipeline pipeline_{nullptr, nullptr};

public:
    VkEngine();

    ~VkEngine();

    void run();

private:
    void update() const;
    void draw();
};
}
