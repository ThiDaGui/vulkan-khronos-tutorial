#pragma once

import vulkan_hpp;

#include <cstdint>
#include <optional>
#include <vector>

struct RequiredQueueFamilyIndices {
    std::optional<uint32_t> graphic_queue = std::nullopt;
    std::optional<uint32_t> present_queue = std::nullopt;

    void Populate(const vk::raii::PhysicalDevice &device, const vk::raii::SurfaceKHR &surface);

    [[nodiscard]] bool isComplete() const;
};

std::vector<const char *> getRequiredInstanceExtensions();

std::vector<const char *> getRequiredInstanceLayers();

std::vector<const char *> getRequiredDeviceExtensions();

void listPhysicalDevices(const std::vector<vk::raii::PhysicalDevice> &physical_devices);

vk::SurfaceFormatKHR chooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& available_surface_formats);

vk::PresentModeKHR choosePresentMode(const std::vector<vk::PresentModeKHR>& available_present_modes);

std::uint32_t chooseMinImageCount(const vk::SurfaceCapabilitiesKHR& surface_capabilities);