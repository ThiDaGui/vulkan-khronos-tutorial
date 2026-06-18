#pragma once

#include <vulkan/vulkan_raii.hpp>

#include "required_queue_family_indices.hh"
#include "window_system.hh"
#include "types/mem_allocator.hh"

namespace vk_tutorial
{
struct Core
{
    vk::raii::Context context{};

    vk::raii::Instance instance{nullptr};
    vk::raii::DebugUtilsMessengerEXT debug_messenger{nullptr};
    vk::raii::PhysicalDevice physical_device{nullptr};
    vk::raii::Device device{nullptr};
    vk_types::MemAllocator vma_allocator{};

    vk::raii::SurfaceKHR surface{nullptr};

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
