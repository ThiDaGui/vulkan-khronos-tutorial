//
// Created by damiendidier on 18/11/2025.
//

#include "vk_engine.hh"

#include <iostream>
#include <memory>
#include <stdexcept>
#include <GLFW/glfw3.h>

#include "vulkan/vk_platform.h"

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
    std::vector<const char *> instance_extensions = window_system_.getRequiredExtensions();
    std::vector<const char *> instance_layers{};

#ifndef NDEBUG
    instance_extensions.push_back(vk::EXTDebugUtilsExtensionName);
    instance_layers.push_back("VK_LAYER_KHRONOS_validation");
#endif

    createInstance(instance_extensions, instance_layers);

#ifndef NDEBUG
    createDebugMessenger();
#endif

    std::vector<const char *> device_extensions{};
    pickPhysicalDevice(device_extensions);
}

void VkEngine::createInstance(
    const std::vector<const char *> &instance_extensions,
    const std::vector<const char *> &instance_layers)
{
    constexpr  vk::ApplicationInfo application_info = {
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

void VkEngine::pickPhysicalDevice(const std::vector<const char *> & device_extensions)
{
    using GradeDeviceType = std::pair<const vk::raii::PhysicalDevice &, uint32_t>;

    std::vector<vk::raii::PhysicalDevice> physical_devices = instance_.enumeratePhysicalDevices();
    if (physical_devices.empty())
        throw std::runtime_error("Failed to find GPU with Vulkan support!");

    std::vector<GradeDeviceType> grades{};
    grades.reserve(physical_devices.size());

    for (const auto &physical_device : physical_devices) {
        grades.emplace_back(physical_device, gradePhysicalDevice(physical_device));
    }
}

uint32_t VkEngine::gradePhysicalDevice(
    const vk::raii::PhysicalDevice &physical_device)
{
    const auto &device_properties = physical_device.getProperties();
    const auto &device_features = physical_device.getFeatures();
    const auto &device_extensions = physical_device.enumerateDeviceExtensionProperties();

    if (device_properties.apiVersion < vk::ApiVersion13)
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

WindowSystem::~WindowSystem()
{
    if (nullptr == window)
        return;
    glfwDestroyWindow(window);
    glfwTerminate();
}

bool WindowSystem::shouldClose() const
{
    return glfwWindowShouldClose(window);
}

}
