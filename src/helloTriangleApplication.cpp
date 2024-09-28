#include "helloTriangleApplication.hh"

#include <GLFW/glfw3.h>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_funcs.hpp>
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_structs.hpp>

#include "vulkanUtils.hh"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::vector<const char *> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char *> deviceExtensions = {
    vk::KHRSwapchainExtensionName
};

VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
              VkDebugUtilsMessageTypeFlagsEXT messageType,
              VkDebugUtilsMessengerCallbackDataEXT const *pCallbackData,
              void * /*pUserData*/)
{
    std::ostringstream message;
    std::string prefix;
    std::string suffix;

    message << "validation layer: ";

    switch (messageType) {
    case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
        message << "validation: ";
        break;
    case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
        message << "general: ";
        break;
    case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
        message << "performance: ";
        break;

    default:
        break;
    }

    message << pCallbackData->pMessage;

#ifdef __linux__
    switch (messageSeverity) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        prefix = "\x1B[33m";
        suffix = "\x1B[0m";
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
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

HelloTriangleApplication::HelloTriangleApplication()
{
    initWindow();
    initVulkan();
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

    if ((window_ = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr))
        == nullptr)
        throw std::runtime_error("failed to create GLFW window!");
}

void HelloTriangleApplication::initVulkan()
{
    createInstance();

#ifndef NDEBUG
    setupDebugMessenger();
#endif
}

void HelloTriangleApplication::createInstance()
{
#ifndef NDEBUG
    // check for validation layers if requested
    {
        std::cout << "Checking for validation layers...";
        if (!checkValidationLayerSupport(validationLayers))
            throw std::runtime_error(
                "validation layers requested, but not available!");
        std::cout << "Done\n";
    }
#endif // NDEBUG

    // create ApplicationInfo struct
    vk::ApplicationInfo applicationInfo{ "Hello Triangle",
                                         vk::makeApiVersion(0, 1, 0, 0),
                                         "No Engine",
                                         vk::makeApiVersion(0, 1, 0, 0),
                                         vk::ApiVersion10 };

    std::vector<const char *> instanceExtensions =
        getRequiredInstanceExtensions();

#ifdef NDEBUG
    vk::InstanceCreateInfo instanceCreateInfo{ {},
                                               &applicationInfo,
                                               {},
                                               {},
                                               static_cast<uint32_t>(
                                                   instanceExtensions.size()),
                                               instanceExtensions.data() };

#else
    // Add validation layers to InstanceCreateInfo struct

    vk::DebugUtilsMessengerCreateInfoEXT debugMessenger{
        {},
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose
            | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
            | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
            | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
            | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
        debugCallback
    };

    vk::InstanceCreateInfo instanceCreateInfo{
        {},
        &applicationInfo,
        static_cast<uint32_t>(validationLayers.size()),
        validationLayers.data(),
        static_cast<uint32_t>(instanceExtensions.size()),
        instanceExtensions.data(),
        &debugMessenger
    };
#endif
    instance_ = vk::createInstance(instanceCreateInfo);
}

void HelloTriangleApplication::setupDebugMessenger()
{
    vk::DebugUtilsMessageSeverityFlagsEXT messageSeverityFlags{
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose
        | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
        | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
    };

    vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags{};

    debugMessenger_ = instance_.createDebugUtilsMessengerEXT(
        vk::DebugUtilsMessengerCreateInfoEXT{
            {}, messageSeverityFlags, messageTypeFlags, nullptr });
}
