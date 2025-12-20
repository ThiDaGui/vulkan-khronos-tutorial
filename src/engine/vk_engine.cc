//
// Created by damiendidier on 18/11/2025.
//

#include "vk_engine.hh"

#include <algorithm>
#include <cstring>
#include <iostream>
#include <unordered_set>
#include <stdexcept>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "required_queue_family_indices.hh"

namespace vk_tutorial {

VKAPI_ATTR vk::Bool32 VKAPI_CALL debugMessageCallback(
    const vk::DebugUtilsMessageSeverityFlagBitsEXT message_severity,
    const vk::DebugUtilsMessageTypeFlagsEXT message_type,
    const vk::DebugUtilsMessengerCallbackDataEXT *p_callback_data,
    void *p_user_data)
{
    std::cerr << "validation layer: " << p_callback_data->pMessage << std::endl;

    return vk::False;
}

VkEngine::VkEngine()
{
    window_system_.init(window_extent.width, window_extent.height, "Vulkan Tutorial");
    initVulkan();
    is_initialized = true;
}

VkEngine::~VkEngine() = default;

void VkEngine::run()
{
    while (!window_system_.shouldClose()) {
        glfwPollEvents();
        draw();
    }
}

void VkEngine::initVulkan()
{
    std::vector<const char *> instance_extensions = WindowSystem::getRequiredExtensions();
    std::vector<const char *> instance_layers{};

#ifndef NDEBUG
    instance_extensions.push_back(vk::EXTDebugUtilsExtensionName);
    instance_layers.push_back("VK_LAYER_KHRONOS_validation");
#endif

    createInstance(instance_extensions, instance_layers);

#ifndef NDEBUG
    createDebugMessenger();
#endif

    surface_ = window_system_.createSurface(instance_);

    pickPhysicalDevice(required_device_extensions);

    createDevice();

    createSwapchain();
}

void VkEngine::createInstance(
    const std::vector<const char *> &instance_extensions,
    const std::vector<const char *> &instance_layers)
{
    constexpr vk::ApplicationInfo application_info = {
        .pApplicationName = "Vulkan Tutorial",
        .applicationVersion = vk::makeVersion(1, 0, 0),
        .pEngineName = "No Engine",
        .engineVersion = vk::makeVersion(1, 0, 0),
        .apiVersion = vk::ApiVersion14,
    };

    const vk::InstanceCreateInfo create_info = {
        .pApplicationInfo = &application_info,
        .enabledLayerCount = static_cast<uint32_t>(instance_layers.size()),
        .ppEnabledLayerNames = instance_layers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(instance_extensions.size()),
        .ppEnabledExtensionNames = instance_extensions.data()
    };

    instance_ = vk::raii::Instance{context_, create_info};
}

void VkEngine::createDebugMessenger()
{
    constexpr vk::DebugUtilsMessageSeverityFlagsEXT severity_flags{
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose
        | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
        | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
    };
    constexpr vk::DebugUtilsMessageTypeFlagsEXT type_flags{
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
        | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance
        | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
    };

    constexpr vk::DebugUtilsMessengerCreateInfoEXT messenger_create_info{
        .messageSeverity = severity_flags,
        .messageType = type_flags,
        .pfnUserCallback = &debugMessageCallback,
    };

    debug_messenger_ = instance_.createDebugUtilsMessengerEXT(messenger_create_info);
}

void VkEngine::pickPhysicalDevice(const std::span<const char * const> device_extensions)
{
    // TODO : Check Vulkan Features
    const std::vector<vk::raii::PhysicalDevice> physical_devices = instance_.enumeratePhysicalDevices();
    if (physical_devices.empty())
        throw std::runtime_error("Failed to find GPU with Vulkan support!");

    std::vector<uint32_t> grades{};
    grades.reserve(physical_devices.size());

    for (const auto &physical_device : physical_devices) {
        grades.emplace_back(gradePhysicalDevice(physical_device, surface_, device_extensions));
    }

    uint32_t best_grade = grades[0];
    uint32_t best_grade_index = 0;
    for (uint32_t i = 1; i < grades.size(); i++) {
        if (grades[i] > best_grade) {
            best_grade = grades[i];
            best_grade_index = i;
        }
    }

    if (best_grade == 0)
        throw std::runtime_error("No GPU support required features!");

    physical_device_ = physical_devices[best_grade_index];
    queue_family_indices_.Populate(physical_device_, surface_);
}

uint32_t VkEngine::gradePhysicalDevice(
    const vk::raii::PhysicalDevice &physical_device,
    const vk::raii::SurfaceKHR &surface,
    const std::span<const char * const> required_extensions)
{
    // TODO : Check if required Vulkan features are supported

    const auto &device_properties = physical_device.getProperties();
    const auto &device_features = physical_device.getFeatures();
    const auto &device_extensions = physical_device.enumerateDeviceExtensionProperties();
    RequiredQueueFamilyIndices required_queue_family_indices{};

    if (device_properties.apiVersion < vk::ApiVersion13)
        return 0;

    for (const char * const required_extension: required_extensions) {
        if (std::ranges::none_of(device_extensions.begin(), device_extensions.end(),
            [required_extension](const vk::ExtensionProperties &device_extension) {
                return std::strcmp(device_extension.extensionName, required_extension) == 0;
            }))
            return 0;
    }

    required_queue_family_indices.Populate(physical_device, surface);
    if (!required_queue_family_indices.isComplete())
        return 0;

    uint32_t score = 0;
    switch (device_properties.deviceType) {
    case vk::PhysicalDeviceType::eDiscreteGpu:
        score += 1000;
        break;

    case vk::PhysicalDeviceType::eIntegratedGpu:
        score += 500;
        break;

    default:
        score += 1;
        break;
    }

    if (device_features.samplerAnisotropy)
        score += static_cast<uint32_t>(device_properties.limits.maxSamplerAnisotropy) * 10;

    return score;
}

void VkEngine::createDevice()
{
    std::unordered_set unique_queue_family_indices = {
        queue_family_indices_.graphics_queue_family.value(),
        queue_family_indices_.present_queue_family.value(),
    };

    std::vector<vk::DeviceQueueCreateInfo> queue_create_infos;
    queue_create_infos.reserve(unique_queue_family_indices.size());

    float priority = 1.0f;
    for (unsigned unique_queue_family_index : unique_queue_family_indices) {
        const vk::DeviceQueueCreateInfo create_info = {
            .queueFamilyIndex = unique_queue_family_index,
            .queueCount = 1,
            .pQueuePriorities = &priority,
        };
        queue_create_infos.emplace_back(create_info);
    }

    const vk::StructureChain<vk::PhysicalDeviceFeatures2,
                             vk::PhysicalDeviceVulkan11Features,
                             vk::PhysicalDeviceVulkan13Features,
                             vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT> feature_chain = {
        {.features = {.samplerAnisotropy = true}},
        {.shaderDrawParameters = true},
        {.synchronization2 = true, .dynamicRendering = true},
        {.extendedDynamicState = true}
    };

    vk::DeviceCreateInfo create_info = {
        .pNext = feature_chain.get(),
        .queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size()),
        .pQueueCreateInfos = queue_create_infos.data(),
        .enabledExtensionCount = static_cast<uint32_t>(required_device_extensions.size()),
        .ppEnabledExtensionNames = required_device_extensions.data(),
    };

    device_ = vk::raii::Device{physical_device_, create_info};
    graphics_queue_ = vk::raii::Queue{device_, queue_family_indices_.graphics_queue_family.value(), 0};
    present_queue_ = vk::raii::Queue{device_, queue_family_indices_.present_queue_family.value(), 0};
}

void VkEngine::createSwapchain()
{
    const vk::SurfaceCapabilitiesKHR surface_capabilities = physical_device_.getSurfaceCapabilitiesKHR(surface_);
    const std::vector<vk::SurfaceFormatKHR> surface_formats = physical_device_.getSurfaceFormatsKHR(surface_);
    const std::vector<vk::PresentModeKHR> present_modes = physical_device_.getSurfacePresentModesKHR(surface_);

    swapchain_extent_ = chooseExtent2D(surface_capabilities);
    swapchain_min_image_count_ = chooseMinImageCount(surface_capabilities);
    const vk::SurfaceFormatKHR surface_format = chooseSurfaceFormat(surface_formats);
    const vk::PresentModeKHR present_mode = choosePresentMode(present_modes);

    vk::SwapchainCreateInfoKHR swapchain_create_info = {
        .surface = surface_,
        .minImageCount = swapchain_min_image_count_,
        .imageFormat = surface_format.format,
        .imageColorSpace = surface_format.colorSpace,
        .imageExtent = swapchain_extent_,
        .imageArrayLayers = 1,
        .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
        .preTransform = surface_capabilities.currentTransform,
        .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
        .presentMode = present_mode,
        .clipped = vk::True,
        .oldSwapchain = nullptr,
    };

    const std::array queue_family_indices = {
        queue_family_indices_.graphics_queue_family.value(),
        queue_family_indices_.present_queue_family.value(),
    };

    if (queue_family_indices_.graphics_queue_family.value() == queue_family_indices_.present_queue_family.value())
        swapchain_create_info.imageSharingMode = vk::SharingMode::eExclusive;
    else {
        swapchain_create_info.imageSharingMode = vk::SharingMode::eConcurrent,
        swapchain_create_info.queueFamilyIndexCount = static_cast<uint32_t>(queue_family_indices.size());
        swapchain_create_info.pQueueFamilyIndices = queue_family_indices.data();
    }

    swapchain_ = vk::raii::SwapchainKHR{device_, swapchain_create_info};
    swapchain_image_format_ = surface_format.format;
    swapchain_images_ = swapchain_.getImages();

    swapchain_image_views_.clear();

    constexpr vk::ImageSubresourceRange subresource_range{
        .aspectMask = vk::ImageAspectFlagBits::eColor,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1,
    };

    vk::ImageViewCreateInfo image_view_create_info{
        .viewType = vk::ImageViewType::e2D,
        .format = swapchain_image_format_,
        .subresourceRange = subresource_range
    };

    for (const auto &swapchain_image: swapchain_images_) {
        image_view_create_info.image = swapchain_image;
        swapchain_image_views_.emplace_back(device_, image_view_create_info);
    }

}

vk::Extent2D VkEngine::chooseExtent2D(const vk::SurfaceCapabilitiesKHR &surface_capabilities) const {
    if (surface_capabilities.currentExtent.width != std::numeric_limits<std::uint32_t>::max())
        return surface_capabilities.currentExtent;

    const auto [width, height] = window_system_.getExtent();

    return {
        std::clamp(width, surface_capabilities.minImageExtent.width,
                                  surface_capabilities.maxImageExtent.width),
        std::clamp(height, surface_capabilities.minImageExtent.height,
                                  surface_capabilities.maxImageExtent.height)
    };
}

uint32_t VkEngine::chooseMinImageCount(const vk::SurfaceCapabilitiesKHR &surface_capabilities)
{
    const uint32_t image_count = surface_capabilities.minImageCount + 1;
    if (surface_capabilities.maxImageCount > 0 && image_count > surface_capabilities.maxImageCount)
        return surface_capabilities.maxImageCount;
    return image_count;
}

vk::SurfaceFormatKHR VkEngine::chooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &surface_formats)
{
    for (const auto &available_surface_format: surface_formats) {
        if (available_surface_format.format == vk::Format::eB8G8R8A8Srgb && available_surface_format.colorSpace ==
            vk::ColorSpaceKHR::eSrgbNonlinear)
            return available_surface_format;
    }
    return surface_formats[0];
}

vk::PresentModeKHR VkEngine::choosePresentMode(const std::vector<vk::PresentModeKHR> &present_modes)
{
    for (const auto & present_mode : present_modes) {
        if (present_mode == vk::PresentModeKHR::eMailbox)
            return present_mode;
    }
    return vk::PresentModeKHR::eFifo;
}

void VkEngine::draw()
{
}

void WindowSystem::init(const uint32_t width, const uint32_t height, const char *name)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(static_cast<int>(width), static_cast<int>(height), name, nullptr, nullptr);
    if (nullptr == window)
        throw std::runtime_error("Failed to create GLFW window!");
}

std::vector<const char *> WindowSystem::getRequiredExtensions()
{
    uint32_t glfw_extension_count = 0;
    const auto glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
    return {glfw_extensions, glfw_extensions + glfw_extension_count};
}

vk::raii::SurfaceKHR WindowSystem::createSurface(const vk::raii::Instance &instance) const {
    VkSurfaceKHR vk_surface;
    if (static_cast<VkResult>(vk::Result::eSuccess) != glfwCreateWindowSurface(*instance, window, nullptr, &vk_surface))
        throw std::runtime_error("failed to create window surface!");
    return {instance, vk_surface};
}

WindowSystem::~WindowSystem()
{
    if (nullptr == window)
        return;
    glfwDestroyWindow(window);
    glfwTerminate();
}

vk::Extent2D WindowSystem::getExtent() const
{
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
}

bool WindowSystem::shouldClose() const
{
    return glfwWindowShouldClose(window);
}

}
