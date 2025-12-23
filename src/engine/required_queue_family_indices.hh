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

    void Populate(const vk::raii::PhysicalDevice &physical_device, const vk::raii::SurfaceKHR &surface);

    [[nodiscard]] bool isComplete() const;

private:
    static std::optional<uint32_t> get_first_queue_index(const std::vector<vk::QueueFamilyProperties> &queue_families,
                                                         vk::QueueFlags flags);

    static std::optional<uint32_t> get_present_queue_index(const std::vector<vk::QueueFamilyProperties> &queue_families,
                                                           const vk::raii::PhysicalDevice &physical_device,
                                                           const vk::raii::SurfaceKHR &surface,
                                                           std::optional<uint32_t> queue_family_hint);
};
} // vk_tutorial
