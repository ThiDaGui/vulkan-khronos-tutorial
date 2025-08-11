import vulkan_hpp;

#include "vulkanUtils.hh"

#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>

void RequiredQueueFamilyIndices::Populate(const vk::raii::PhysicalDevice &device, const vk::raii::SurfaceKHR &surface) {
    const auto queues_family_properties = device.getQueueFamilyProperties();

    for (uint32_t i = 0; i < queues_family_properties.size(); i++) {
        if ((queues_family_properties[i].queueFlags & vk::QueueFlagBits::eGraphics) != vk::QueueFlags(0))
            graphic_queue = std::make_optional<std::uint32_t>(i);
        if (device.getSurfaceSupportKHR(i, surface))
            present_queue = std::make_optional<uint32_t>(i);
    }
}

[[nodiscard]] bool RequiredQueueFamilyIndices::isComplete() const {
    return graphic_queue.has_value() && present_queue.has_value();
}

template<typename T>
void printIter(const T &t) {
    for (const auto &iter: t)
        std::cout << '\t' << iter << std::endl;
}

void listRequiredInstanceExtensions(
    const std::vector<const char *> &extensions) {
    std::cout << "required extensions:" << std::endl;

    printIter(extensions);
}

std::vector<const char *> getRequiredInstanceExtensions() {
    uint32_t glfwExtensionCount = 0;
    const auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char *> extensions(glfwExtensions,
                                         glfwExtensions + glfwExtensionCount);

#ifndef NDEBUG
    extensions.push_back(vk::EXTDebugUtilsExtensionName);
    listRequiredInstanceExtensions(extensions);
#endif

    return extensions;
}

void listRequiredInstanceLayers(const std::vector<const char *> &layers) {
    std::cout << "required layers:" << std::endl;

    printIter(layers);
}

std::vector<const char *> getRequiredInstanceLayers() {
    std::vector<const char *> layers;
#ifndef NDEBUG
    layers.push_back("VK_LAYER_KHRONOS_validation");
    listRequiredInstanceLayers(layers);
#endif

    return layers;
}

void listRequiredDeviceExtensions(const std::vector<const char *> &device_extensions) {
    std::cout << "required device extensions:" << std::endl;

    printIter(device_extensions);
}

std::vector<const char *> getRequiredDeviceExtensions() {
    static const std::vector<const char *> required_device_extensions{
        vk::KHRSwapchainExtensionName
    };

#ifndef NDEBUG
    static bool first = true;
    if (first) {
        first = false;
        listRequiredDeviceExtensions(required_device_extensions);
    }
#endif

    return required_device_extensions;
}

void listPhysicalDevices(const std::vector<vk::raii::PhysicalDevice> &physical_devices) {
    std::cout << "Physical devices:" << std::endl;
    for (const auto &physical_device: physical_devices) {
        const char *device_name = physical_device.getProperties().deviceName;
        std::cout << '\t' << device_name << std::endl;
    }
}

vk::SurfaceFormatKHR chooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& available_surface_formats) {
    for (const auto & available_surface_format: available_surface_formats) {
        if (available_surface_format.format == vk::Format::eB8G8R8A8Srgb && available_surface_format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
            return available_surface_format;
    }
    return available_surface_formats[0];
}

vk::PresentModeKHR choosePresentMode(const std::vector<vk::PresentModeKHR>& available_present_modes) {
    for (const auto & available_present_mode: available_present_modes) {
        if (available_present_mode == vk::PresentModeKHR::eMailbox)
            return available_present_mode;
    }
    return vk::PresentModeKHR::eFifo;
}

std::uint32_t chooseMinImageCount(const vk::SurfaceCapabilitiesKHR &surface_capabilities) {
    const std::uint32_t image_count = surface_capabilities.minImageCount + 1;
    const std::uint32_t max_image_count = surface_capabilities.maxImageCount;
    if (max_image_count > 0 && image_count > max_image_count)
        return max_image_count;
    return image_count;
}
