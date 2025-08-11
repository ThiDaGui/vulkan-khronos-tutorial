import vulkan_hpp;

#include "helloTriangleApplication.hh"

#include <algorithm>
#include <bits/ranges_algo.h>
#include <cstring>
#include <iostream>
#include <set>
#include <sstream>

#include "vulkanUtils.hh"


VKAPI_ATTR vk::Bool32 VKAPI_CALL
debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
              vk::DebugUtilsMessageTypeFlagsEXT messageType,
              vk::DebugUtilsMessengerCallbackDataEXT const *pCallbackData,
              void *pUserData) {
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

    if (messageSeverity > vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
        std::cout << prefix << message.str() << suffix << std::endl;

    return vk::False;
}

HelloTriangleApplicationCpp::HelloTriangleApplicationCpp(GLFWwindow *window)
    : window_(window) {
    initVulkan();
}

HelloTriangleApplicationCpp::~HelloTriangleApplicationCpp() {
    cleanup();
}

void HelloTriangleApplicationCpp::run() const {
    while (!glfwWindowShouldClose(window_)) {
        glfwPollEvents();
    }
}

void HelloTriangleApplicationCpp::initWindow() {
    if (!glfwInit())
        throw std::runtime_error("failed to initialize GLFW!");

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    if (nullptr == (window_ = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr)))
        throw std::runtime_error("failed to create GLFW window!");
}

void HelloTriangleApplicationCpp::initVulkan() {
    createInstance();

#ifndef NDEBUG
    setupDebugMessenger();
#endif

    createSurface();

    pickPhysicalDevice();

    createLogicalDevice();

    createSwapChain();
}

void HelloTriangleApplicationCpp::cleanup() {
}

void HelloTriangleApplicationCpp::createInstance() {
    constexpr vk::ApplicationInfo applicationInfo{
        .pApplicationName = "Hello Triangle",
        .applicationVersion = vk::makeVersion(1, 0, 0),
        .pEngineName = "No Engine",
        .engineVersion = vk::makeVersion(1, 0, 0),
        .apiVersion = vk::ApiVersion14,
    };

    auto requiredExtensions = getRequiredInstanceExtensions();
    auto extensionProperties = context_.enumerateInstanceExtensionProperties();

    for (const auto &requiredExtension: requiredExtensions) {
        if (std::ranges::none_of(extensionProperties,
                                 [requiredExtension](auto const &extensionProperty) {
                                     return std::strcmp(extensionProperty.extensionName, requiredExtension) == 0;
                                 })) {
            throw std::runtime_error("Required GLFW extension not supported: " + std::string(requiredExtension));
        }
    }

    std::vector<char const *> requiredLayers = getRequiredInstanceLayers();
    auto layerProperties = context_.enumerateInstanceLayerProperties();

    for (const auto &requiredLayer: requiredLayers) {
        if (std::ranges::none_of(layerProperties,
                                 [requiredLayer](auto const &layerProperty) {
                                     return std::strcmp(layerProperty.layerName, requiredLayer) == 0;
                                 })) {
            throw std::runtime_error("Required layer extension not supported: " + std::string(requiredLayer));
        }
    }

    const vk::InstanceCreateInfo createInfo{
        .pApplicationInfo = &applicationInfo,
        .enabledLayerCount = static_cast<uint32_t>(requiredLayers.size()),
        .ppEnabledLayerNames = requiredLayers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size()),
        .ppEnabledExtensionNames = requiredExtensions.data(),
    };

    instance_ = vk::raii::Instance(context_, createInfo);
}

void HelloTriangleApplicationCpp::createSurface() {
    VkSurfaceKHR surface;
    if (static_cast<VkResult>(vk::Result::eSuccess) != glfwCreateWindowSurface(*instance_, window_, nullptr, &surface))
        throw std::runtime_error("Failed to create window surface!");
    surface_ = vk::raii::SurfaceKHR(instance_, surface);
}

void HelloTriangleApplicationCpp::pickPhysicalDevice() {
    const auto devices = instance_.enumeratePhysicalDevices();
    if (devices.empty())
        throw std::runtime_error("Failed to find GPUs with vulkan support!");

    const auto &required_device_extensions = getRequiredDeviceExtensions();

    listPhysicalDevices(devices);

    bool is_suitable = false;
    for (const auto &device: devices) {
        const auto device_properties = device.getProperties();

        const auto device_extensions = device.enumerateDeviceExtensionProperties();


        const bool is_vk13_supported = device_properties.apiVersion >= vk::ApiVersion13;
        RequiredQueueFamilyIndices family_indices{};
        family_indices.Populate(device, surface_);

        bool is_device_extensions_supported = true;

        for (const auto &required_device_extension: required_device_extensions) {
            const bool found =
                std::ranges::any_of(
                    device_extensions,
                    [required_device_extension](const auto& device_extension)
                    {return std::strcmp(device_extension.extensionName, required_device_extension) == 0;});
            is_device_extensions_supported = is_device_extensions_supported && found;
        }
        is_suitable =
                is_vk13_supported &&
                family_indices.isComplete() &&
                is_device_extensions_supported;
        if (is_suitable) {
            physical_device_ = device;
            queue_family_indices_ = family_indices;

            break;
        }
    }
    if (!is_suitable)
        throw std::runtime_error("Failed to find a suitable GPU!");

#ifndef NDEBUG
    std::cout << "Selected Physical Device: " << physical_device_.getProperties().deviceName << std::endl;
#endif
}

void HelloTriangleApplicationCpp::createLogicalDevice() {
    std::vector<vk::DeviceQueueCreateInfo> device_queue_create_infos{};

    const std::set unique_queue_indices{queue_family_indices_.graphic_queue.value()};
    const auto required_device_extensions = getRequiredDeviceExtensions();
    float queue_priority = 1.0f;

    for (const auto &queue_index : unique_queue_indices) {
        const vk::DeviceQueueCreateInfo device_queue_create_info{
            .queueFamilyIndex = queue_index,
            .queueCount = 1,
            .pQueuePriorities = &queue_priority
        };
        device_queue_create_infos.emplace_back(device_queue_create_info);
    }

    const vk::DeviceCreateInfo device_create_info{
        .queueCreateInfoCount = static_cast<uint32_t>(device_queue_create_infos.size()),
        .pQueueCreateInfos = device_queue_create_infos.data(),
        .enabledExtensionCount = static_cast<uint32_t>(required_device_extensions.size()),
        .ppEnabledExtensionNames = required_device_extensions.data(),
    };

    device_ = vk::raii::Device(physical_device_, device_create_info);
    graphic_queue = vk::raii::Queue(device_, queue_family_indices_.graphic_queue.value(), 0);
    present_queue = vk::raii::Queue(device_, queue_family_indices_.present_queue.value(), 0);
}

void HelloTriangleApplicationCpp::createSwapChain() {
    const vk::SurfaceCapabilitiesKHR surface_capabilities = physical_device_.getSurfaceCapabilitiesKHR(surface_);

    swapchain_extent_ = chooseExtent2D(surface_capabilities);
    const vk::SurfaceFormatKHR surface_format = chooseSurfaceFormat(physical_device_.getSurfaceFormatsKHR(surface_));
    const vk::PresentModeKHR present_mode = choosePresentMode(physical_device_.getSurfacePresentModesKHR(surface_));

    const std::uint32_t min_image_count = chooseMinImageCount(surface_capabilities);

    vk::SwapchainCreateInfoKHR swapchain_create_info{
        .surface = surface_,
        .minImageCount = min_image_count,
        .imageFormat = surface_format.format,
        .imageColorSpace = surface_format.colorSpace,
        .imageExtent = swapchain_extent_,
        .imageArrayLayers = 1,
        .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
        .imageSharingMode = vk::SharingMode::eExclusive,
        .preTransform = surface_capabilities.currentTransform,
        .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
        .presentMode = present_mode,
        .clipped = vk::True,
        .oldSwapchain = nullptr
    };

    const uint32_t queue_family_indices[]{
        queue_family_indices_.graphic_queue.value(),
        queue_family_indices_.present_queue.value(),
    };


    if (queue_family_indices_.graphic_queue.value() == queue_family_indices_.present_queue.value()) {
        swapchain_create_info.imageSharingMode = vk::SharingMode::eExclusive;
    }
    else {
        swapchain_create_info.imageSharingMode = vk::SharingMode::eConcurrent;
        swapchain_create_info.queueFamilyIndexCount = 2;
        swapchain_create_info.pQueueFamilyIndices = queue_family_indices;
    }

    swapchain_ = vk::raii::SwapchainKHR(device_, swapchain_create_info);
    swapchain_images_ = swapchain_.getImages();
    swapchain_image_format_ = surface_format.format;
}


void HelloTriangleApplicationCpp::setupDebugMessenger() {
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
        .pfnUserCallback = &debugCallback
    };

    debug_messenger_ = instance_.createDebugUtilsMessengerEXT(messenger_create_info);
}



vk::Extent2D HelloTriangleApplicationCpp::chooseExtent2D(const vk::SurfaceCapabilitiesKHR& surface_capabilities) const {
    if (surface_capabilities.currentExtent.width != std::numeric_limits<std::uint32_t>::max())
        return surface_capabilities.currentExtent;

    std::int32_t width, height;
    glfwGetFramebufferSize(window_, &width, &height);

    return {
        std::clamp<std::uint32_t>(width, surface_capabilities.minImageExtent.width, surface_capabilities.maxImageExtent.width),
        std::clamp<std::uint32_t>(height, surface_capabilities.minImageExtent.height, surface_capabilities.maxImageExtent.height)
    };
}