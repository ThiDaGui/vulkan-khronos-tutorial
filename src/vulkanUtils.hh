#pragma once

import vulkan_hpp;

#include <filesystem>
#include <optional>
#include <vector>

struct RequiredQueueFamilyIndices {
    std::optional<uint32_t> graphic_queue = std::nullopt;
    std::optional<uint32_t> present_queue = std::nullopt;

    void Populate(const vk::raii::PhysicalDevice &device,
                  const vk::raii::SurfaceKHR &surface);

    [[nodiscard]] bool isComplete() const;
};

std::vector<const char *> getRequiredInstanceExtensions();

std::vector<const char *> getRequiredInstanceLayers();

std::vector<const char *> getRequiredDeviceExtensions();

void listPhysicalDevices(
    const std::vector<vk::raii::PhysicalDevice> &physical_devices);

vk::SurfaceFormatKHR chooseSurfaceFormat(
    const std::vector<vk::SurfaceFormatKHR> &available_surface_formats);

vk::PresentModeKHR choosePresentMode(
    const std::vector<vk::PresentModeKHR> &available_present_modes);

std::uint32_t chooseMinImageCount(
    const vk::SurfaceCapabilitiesKHR &surface_capabilities);

std::vector<char> readShader(const std::filesystem::path &file_path);

void transitionImageLayout(const vk::raii::CommandBuffer &command_buffer,
                           const vk::Image &image,
                           uint32_t mip_levels,
                           vk::Format image_format,
                           vk::ImageLayout old_layout,
                           vk::ImageLayout new_layout,
                           vk::PipelineStageFlags2 src_stage_mask,
                           vk::AccessFlags2 src_access_mask,
                           vk::PipelineStageFlags2 dst_stage_mask,
                           vk::AccessFlags2 dst_access_mask);


uint32_t findMemoryTypeIndex(
    const vk::PhysicalDeviceMemoryProperties &memory_properties,
    std::uint32_t memory_type_bits,
    vk::MemoryPropertyFlags property_flags);

vk::Format findDepthFormat(const vk::raii::PhysicalDevice &physical_device);

void generateMips(
    const vk::raii::CommandBuffer &command_buffer,
    const vk::raii::Image& image,
    int32_t image_width,
    int32_t image_height, uint32_t mip_levels);

void listUsableSampleCounts(vk::SampleCountFlags samples, vk::SampleCountFlagBits used_sample);