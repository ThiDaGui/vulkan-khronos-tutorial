#pragma once

#include <vulkan/vulkan_raii.hpp>
#include <vk_mem_alloc/vma_usage.hh>

#include "required_queue_family_indices.hh"
#include "window_system.hh"

namespace vk_tutorial
{
struct Core
{
    vk::raii::Context context_{};

    vk::raii::Instance instance_{nullptr};
    vk::raii::DebugUtilsMessengerEXT debug_messenger_{nullptr};
    vk::raii::PhysicalDevice physical_device_{nullptr};
    vk::raii::Device device_{nullptr};
    VmaAllocator vma_allocator{};

    vk::raii::SurfaceKHR surface_{nullptr};

    RequiredQueueFamilyIndices queue_family_indices{};
    vk::raii::Queue graphics_queue{nullptr};
    vk::raii::Queue present_queue{nullptr};

    vk::raii::CommandPool graphics_command_pool{nullptr};

    void init(std::span<const char* const> instance_extensions,
              std::span<const char* const> instance_layers,
              std::span<const char* const> device_extensions,
              WindowSystem& window_system);
};
}
