#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <map>
#include <optional>
#include <set>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <algorithm>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::vector<const char *> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char *> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        VkDebugUtilsMessengerCallbackDataEXT const *pCallbackData,
        void */*pUserData*/) {

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

    std::cout << prefix << message.str() << suffix << std::endl;

    return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
                                      const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                      const VkAllocationCallbacks *pAllocator,
                                      VkDebugUtilsMessengerEXT *pDebugMessenger) {
    auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(
        VkInstance instance,
        VkDebugUtilsMessengerEXT debugMessenger,
        const VkAllocationCallbacks *pAllocator) {
    auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
    if (func != nullptr) {
        return func(instance, debugMessenger, pAllocator);
    }
}

void listRequiredInstanceExtensions(const std::vector<const char *> &extensions) {
    std::cout << "required extensions:\n";

    for (const auto extension: extensions) {
        std::cout << '\t' << extension << std::endl;
    }
}

class HelloTriangleApplication {

    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        [[nodiscard]] bool isComplete() const {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities{};
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

private:
    GLFWwindow *window_ = nullptr;
    VkInstance instance_ = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT debugMessenger_ = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice_ = VK_NULL_HANDLE;
    VkDevice device_ = VK_NULL_HANDLE;
    VkSurfaceKHR surface_ = VK_NULL_HANDLE;
    VkQueue graphicsQueue_ = VK_NULL_HANDLE;
    VkQueue presentQueue_ = VK_NULL_HANDLE;
    VkSwapchainKHR swapChain_ = VK_NULL_HANDLE;
    std::vector<VkImage> swapChainImages_;
    //VkFormat swapChainImageFormat_;
    //VkExtent2D swapChainExtent_;


public:
    void run() {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    void initWindow() {
        if (!glfwInit())
            throw std::runtime_error("Failed to initialize GLFW!");

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        if ((window_ = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr)) == nullptr)
            throw std::runtime_error("Failed to create GLFW window!");
    }

    void initVulkan() {
        createInstance();
#ifndef NDEBUG
        setupDebugMessenger();
#endif
        createSurface();
        pickPhysicalDevice();
        createLogicalDevice();
        createSwapChain();

    }

    void mainLoop() {
        while (!glfwWindowShouldClose(window_)) {
            glfwPollEvents();
        }
    }

    void cleanup() {
        vkDestroySwapchainKHR(device_, swapChain_, nullptr);
#ifndef NDEBUG
        DestroyDebugUtilsMessengerEXT(instance_, debugMessenger_, nullptr);
#endif
        vkDestroyDevice(device_, nullptr);
        vkDestroySurfaceKHR(instance_, surface_, nullptr);
        vkDestroyInstance(instance_, nullptr);

        glfwDestroyWindow(window_);

        glfwTerminate();
    }


    static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo) {
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
        createInfo.pUserData = nullptr; // Optional
    }

    void setupDebugMessenger() {
        VkDebugUtilsMessengerCreateInfoEXT createInfo{};
        populateDebugMessengerCreateInfo(createInfo);

        if (CreateDebugUtilsMessengerEXT(
                instance_,
                &createInfo,
                nullptr,
                &debugMessenger_) != VK_SUCCESS)
            throw std::runtime_error("failed to set up a debug messenger!");
    }

    static std::vector<const char *> getRequiredInstanceExtensions() {
        uint32_t glfwExtensionCount = 0;
        const char **glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

#ifndef NDEBUG
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        listRequiredInstanceExtensions(extensions);
#endif

        return extensions;
    }

    static bool checkValidationLayerSupport() {
        uint32_t layerCount;
        if (vkEnumerateInstanceLayerProperties(&layerCount, nullptr) != VK_SUCCESS)
            throw std::runtime_error("error while enumerating instance layer properties");

        std::vector<VkLayerProperties> availableLayers(layerCount);
        if (vkEnumerateInstanceLayerProperties(&layerCount,
                                               availableLayers.data()) != VK_SUCCESS)
            throw std::runtime_error("error while enumerating instance layer properties");

        std::set<std::string> requiredLayers(validationLayers.begin(), validationLayers.end());
        for (const auto &layerProperty: availableLayers) {
            requiredLayers.erase(layerProperty.layerName);
        }
        return requiredLayers.empty();
    }

    static bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> extensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, extensions.data());

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
        for (const auto &extension: extensions) {
            requiredExtensions.erase(extension.extensionName);
        }
        return requiredExtensions.empty();
    }

    static void listAvailableExtensions() {
        uint32_t extensionCount = 0;
        if (vkEnumerateInstanceExtensionProperties(
                nullptr,
                &extensionCount,
                nullptr) != VK_SUCCESS)
            throw std::runtime_error("Error while enumerating Instance extension properties");

        std::vector<VkExtensionProperties> extensions(extensionCount);
        if (vkEnumerateInstanceExtensionProperties(
                nullptr,
                &extensionCount,
                extensions.data()) !=
            VK_SUCCESS)
            throw std::runtime_error("Error while enumerating Instance extension properties");

        std::cout << "available extensions:\n";

        for (const auto &extension: extensions) {
            std::cout << '\t' << extension.extensionName << std::endl;
        }
    }

    void createInstance() {
#ifndef NDEBUG
        //----- check for validation layers if requested -----
        {
            std::cout << "Checking for validation layers...";
            if (!checkValidationLayerSupport()) {
                throw std::runtime_error("validation layers requested, but not available!");
            }
            std::cout << " Done\n";
        }
#endif

        //----- create ApplicationInfo struct -----
        VkApplicationInfo applicationInfo{};
        applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        applicationInfo.pApplicationName = "Hello Triangle";
        applicationInfo.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
        applicationInfo.pEngineName = "No Engine";
        applicationInfo.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
        applicationInfo.apiVersion = VK_API_VERSION_1_0;

        //----- create InstanceCreateInfo struct and check for required instanceExtensions -----
        VkInstanceCreateInfo createInfo{};

        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &applicationInfo;

        std::vector<const char *> instanceExtensions = getRequiredInstanceExtensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
        createInfo.ppEnabledExtensionNames = instanceExtensions.data();

#ifndef NDEBUG
        //----- Add validation layers to InstanceCreateInfo struct -----
        VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoExt{};

        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        populateDebugMessengerCreateInfo(debugUtilsMessengerCreateInfoExt);
        createInfo.pNext = &debugUtilsMessengerCreateInfoExt;
#else
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
#endif

        if (vkCreateInstance(&createInfo, nullptr, &instance_) != VK_SUCCESS) {
            throw std::runtime_error("failed to create instance:");
        }
    }

    void createSurface() {
        if (glfwCreateWindowSurface(instance_, window_, nullptr, &surface_) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create window surface!");
        }
    }

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice physicalDevice) {
        SwapChainSupportDetails details;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface_, &details.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface_, &formatCount, nullptr);
        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface_, &formatCount, details.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface_, &presentModeCount, nullptr);
        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface_, &presentModeCount,
                                                      details.presentModes.data());
        }

        return details;
    }

    void pickPhysicalDevice() {
        uint32_t deviceCount = 0;
        if (vkEnumeratePhysicalDevices(instance_, &deviceCount, nullptr) != VK_SUCCESS)
            throw std::runtime_error("Could not enumerate physical Devices!");

        if (deviceCount == 0)
            throw std::runtime_error("Failed to find GPUs with Vulkan support!");

        std::vector<VkPhysicalDevice> devices(deviceCount);
        if (vkEnumeratePhysicalDevices(instance_, &deviceCount, devices.data()) != VK_SUCCESS)
            throw std::runtime_error("Could not enumerate physical Devices!");

        for (const auto &device: devices) {
            if (isDeviceSuitable(device, surface_)) {
                physicalDevice_ = device;
                break;
            }

        }
        if (physicalDevice_ == VK_NULL_HANDLE) {
            throw std::runtime_error("Failed to find suitable GPU!");
        }

    }

    static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
        QueueFamilyIndices indices{};

        VkBool32 presentSupport = VK_FALSE;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        uint32_t i = 0;
        for (const auto &queueFamily: queueFamilies) {
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i;
            }
            if (presentSupport) {
                indices.presentFamily = i;
            }
            if (indices.isComplete()) {
                break;
            }
            i++;
        }

        return indices;
    }

    bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface) {
        QueueFamilyIndices indices = findQueueFamilies(device, surface);

        bool extensionsSupported = checkDeviceExtensionSupport(device);

        bool swapChainAdequate = false;
        if (extensionsSupported) {
            SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
            swapChainAdequate = !swapChainSupport.presentModes.empty() && !swapChainSupport.formats.empty();
        }
        return indices.isComplete() && extensionsSupported && swapChainAdequate;
    }

    static uint32_t rateDeviceSuitability(VkPhysicalDevice device) {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);

        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

        uint32_t score = 0;
        if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            score += 1000;

        score += deviceProperties.limits.maxImageDimension2D;

        if (!deviceFeatures.geometryShader)
            return 0;

        return score;
    }

    void createLogicalDevice() {
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice_, surface_);

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies{indices.graphicsFamily.value(), indices.presentFamily.value()};

        float queuePriority = 1.0f;
        for (uint32_t queueFamily: uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo =
                    {VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                     nullptr,
                     0,
                     queueFamily,
                     1,
                     &queuePriority};
            queueCreateInfos.emplace_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures{};

        VkDeviceCreateInfo deviceCreateInfo{};
        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
        deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());

        deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
        deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

#ifndef NDEBUG
        deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
#else
        deviceCreateInfo.enabledLayerCount = 0;
#endif
        if (vkCreateDevice(physicalDevice_, &deviceCreateInfo, nullptr, &device_) != VK_SUCCESS)
            throw std::runtime_error("Failed to create logical device!");

        vkGetDeviceQueue(device_, indices.graphicsFamily.value(), 0, &graphicsQueue_);
        vkGetDeviceQueue(device_, indices.presentFamily.value(), 0, &presentQueue_);
    }

    static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) {
        for (const auto &availableFormat: availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
                availableFormat.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR)
                return availableFormat;
        }
        return availableFormats[0];
    }

    static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) {
        for (const auto &availablePresentMode: availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
                return availablePresentMode;
        }
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) {
        if (capabilities.currentExtent.height != std::numeric_limits<uint32_t>::max() ||
            capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
            return capabilities.currentExtent;

        int width, height;
        glfwGetFramebufferSize(window_, &width, &height);
        VkExtent2D actualExtent{static_cast<uint32_t>(width),
                                static_cast<uint32_t>(height)};

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width,
                                        capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height,
                                         capabilities.maxImageExtent.height);

        return actualExtent;
    }

    void createSwapChain() {
        SwapChainSupportDetails swapChainSupportDetails = querySwapChainSupport(physicalDevice_);

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupportDetails.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupportDetails.presentModes);
        VkExtent2D extent2D = chooseSwapExtent(swapChainSupportDetails.capabilities);

        uint32_t imageCount = swapChainSupportDetails.capabilities.minImageCount + 1;
        if (imageCount > 0 && swapChainSupportDetails.capabilities.maxImageCount > 0 && imageCount > swapChainSupportDetails.capabilities.maxImageCount)
            imageCount = swapChainSupportDetails.capabilities.maxImageCount;

        VkSwapchainCreateInfoKHR createInfo{};

        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface_;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent2D;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices indices = findQueueFamilies(physicalDevice_, surface_);
        uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

        if (indices.graphicsFamily != indices.presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        } else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0;
            createInfo.pQueueFamilyIndices = nullptr;
        }

        createInfo.preTransform = swapChainSupportDetails.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = VK_NULL_HANDLE;

        if (vkCreateSwapchainKHR(device_, &createInfo, nullptr, &swapChain_) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create swap chain!");
        }

        //vkGetSwapchainImagesKHR(device_, swapChain_, &imageCount, nullptr);
        //swapChainImages_.resize(imageCount);
        //vkGetSwapchainImagesKHR(device_, swapChain_, &imageCount, swapChainImages_.data());
//
        //swapChainImageFormat_ = surfaceFormat.format;
        //swapChainExtent_ = extent2D;
    }
};


int main() {
    HelloTriangleApplication app;

#ifdef NDEBUG
    std::cout << "Release binary\n";
#else
    std::cout << "Debug binary\n";
#endif

    try {
        app.run();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}