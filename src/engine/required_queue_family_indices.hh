#pragma once

#include <optional>
#include <vector>

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

namespace vk_tutorial
{
struct RequiredQueueFamilyIndices
{
    std::optional<std::uint32_t> graphics_queue_family{};
    std::optional<std::uint32_t> present_queue_family{};

    void populate(const vk::raii::PhysicalDevice& physical_device, const vk::raii::SurfaceKHR& surface);

    [[nodiscard]] bool isComplete() const;

private:
    static std::optional<uint32_t> getFirstQueueIndex(const std::vector<vk::QueueFamilyProperties>& queue_families,
                                                      vk::QueueFlags flags);

    static std::optional<uint32_t> getPresentQueueIndex(const std::vector<vk::QueueFamilyProperties>& queue_families,
                                                        const vk::raii::PhysicalDevice& physical_device,
                                                        const vk::raii::SurfaceKHR& surface,
                                                        std::optional<uint32_t> queue_family_hint);
};
} // vk_tutorial
