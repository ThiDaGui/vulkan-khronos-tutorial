#include "helloTriangleApplication.hh"

import vulkan_hpp;

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstring>
#include <iostream>
#include <bits/ranges_algo.h>
#include <sstream>

#include "vulkanUtils.hh"

constexpr uint32_t WIDTH = 800;
constexpr uint32_t HEIGHT = 600;

const std::vector<const char *> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char *> deviceExtensions = {
    vk::KHRSwapchainExtensionName
};

/*
[[maybe_unused]] VKAPI_ATTR vk::Bool32 VKAPI_CALL
debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
              vk::DebugUtilsMessageTypeFlagsEXT messageType,
              vk::DebugUtilsMessengerCallbackDataEXT const *pCallbackData,
              void * pUserData)
{
    std::ostringstream message{};
    std::string prefix;
    std::string suffix;

    message << "validation layer: ";

    if (messageType | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation)
        message << "validation: ";
    if (messageType | vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral)
        message << "general: ";
    if (messageType | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance)
        message << "performance: ";

    message << pCallbackData->pMessage;

#ifdef __linux__
    switch (messageSeverity) {
    case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning:
        prefix = "\x1B[33m";
        suffix = "\x1B[0m";
        break;
    case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:
        prefix = "\x1B[31m";
        suffix = "\x1B[0m";
        break;

    default:
        break;
    }
#endif //__linux__

    std::cout << prefix << message.str() << suffix << std::endl;

    return VK_FALSE;
}
*/

HelloTriangleApplication::HelloTriangleApplication()
    : window_{nullptr}
{
    initWindow();
    initVulkan();
}

HelloTriangleApplication::~HelloTriangleApplication()
{
    cleanup();
}

void HelloTriangleApplication::run()
{
    while (!glfwWindowShouldClose(window_)) {
        glfwPollEvents();
    }
}

void HelloTriangleApplication::initWindow()
{
    if (!glfwInit())
        throw std::runtime_error("failed to initialize GLFW!");

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    if (nullptr == (window_ = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr)))
        throw std::runtime_error("failed to create GLFW window!");
}

void HelloTriangleApplication::initVulkan()
{
    createInstance();
}

void HelloTriangleApplication::cleanup()
{
    glfwDestroyWindow(window_);

    glfwTerminate();
}

void HelloTriangleApplication::createInstance()
{
    constexpr vk::ApplicationInfo applicationInfo{
        .pApplicationName = "Hello Triangle",
        .applicationVersion = vk::makeVersion(1, 0, 0),
        .pEngineName = "No Engine",
        .engineVersion = vk::makeVersion(1, 0, 0),
        .apiVersion = vk::ApiVersion14,
    };

    auto requiredExtensions = getRequiredInstanceExtensions();
    auto extensionProperties = context_.enumerateInstanceExtensionProperties();

    for (uint32_t i = 0; i < requiredExtensions.size(); ++i)
    {
        if (std::ranges::none_of(extensionProperties,
                                 [extension = requiredExtensions[i]](auto const& extensionProperty)
                                 { return std::strcmp(extensionProperty.extensionName, extension) == 0; }))
        {
            throw std::runtime_error("Required GLFW extension not supported: " + std::string(requiredExtensions[i]));
        }
    }

    vk::InstanceCreateInfo createInfo{
            .pApplicationInfo = &applicationInfo,
            .enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size()),
            .ppEnabledExtensionNames = requiredExtensions.data(),
    };

    instance_ = vk::raii::Instance(context_, createInfo);
}