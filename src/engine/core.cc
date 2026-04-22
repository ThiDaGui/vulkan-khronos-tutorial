//
// Created by damiendidier on 12/29/25.
//

#include "core.hh"

#include <iostream>
#include <functional>
#include <unordered_set>

#include "required_queue_family_indices.hh"

namespace vk_tutorial
{
uint32_t grade_physical_device(const vk::raii::PhysicalDevice& physical_device,
                               const vk::raii::SurfaceKHR& surface,
                               const std::span<const char* const> required_extensions)
{
    {
        // TODO : Check if required Vulkan features are supported

        const auto& device_properties = physical_device.getProperties();
        const auto& device_features = physical_device.getFeatures();
        const auto& device_extensions = physical_device.enumerateDeviceExtensionProperties();
        RequiredQueueFamilyIndices required_queue_family_indices{};

        if (device_properties.apiVersion < vk::ApiVersion13)
            return 0;

        for (const char* const required_extension : required_extensions)
        {
            if (std::ranges::none_of(device_extensions.begin(), device_extensions.end(),
                                     [required_extension](const vk::ExtensionProperties& device_extension)
                                     {
                                         return std::strcmp(device_extension.extensionName, required_extension) == 0;
                                     }))
                return 0;
        }

        required_queue_family_indices.Populate(physical_device, surface);
        if (!required_queue_family_indices.isComplete())
            return 0;

        uint32_t score = 0;
        switch (device_properties.deviceType)
        {
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
}

VKAPI_ATTR vk::Bool32 VKAPI_CALL debugMessageCallback(
    const vk::DebugUtilsMessageSeverityFlagBitsEXT message_severity,
    const vk::DebugUtilsMessageTypeFlagsEXT message_type,
    const vk::DebugUtilsMessengerCallbackDataEXT* p_callback_data,
    void* p_user_data)
{
    switch (message_severity)
    {
    case vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose:
    case vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo:
    case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning:
        std::cout << vk::to_string(message_severity) << ' ' << vk::to_string(message_type) << ' ' << p_callback_data->
            pMessage << std::endl;
        break;
    case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:
        std::cerr << vk::to_string(message_severity) << ' ' << vk::to_string(message_type) << ' ' << p_callback_data->
            pMessage << std::endl;
        break;
    }

    return vk::False;
}

void Core::init(std::span<const char* const> instance_extensions,
                std::span<const char* const> instance_layers,
                std::span<const char* const> device_extensions,
                WindowSystem& window_system)
{
    // Create Instance
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

    //Create DebugMessenger
#ifndef NDEBUG
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
#endif

    // Create Surface
    surface_ = window_system.createSurface(instance_);

    // Create device
    {
        // TODO : Check Vulkan Features
        const std::vector<vk::raii::PhysicalDevice> physical_devices = instance_.enumeratePhysicalDevices();
        if (physical_devices.empty())
            throw std::runtime_error("Failed to find GPU with Vulkan support!");

        std::vector<uint32_t> grades{};
        grades.reserve(physical_devices.size());

        for (const auto& physical_device : physical_devices)
        {
            grades.emplace_back(grade_physical_device(physical_device, surface_, device_extensions));
        }

        uint32_t best_grade = grades[0];
        uint32_t best_grade_index = 0;
        for (uint32_t i = 1; i < grades.size(); i++)
        {
            if (grades[i] > best_grade)
            {
                best_grade = grades[i];
                best_grade_index = i;
            }
        }

        if (best_grade == 0)
            throw std::runtime_error("No GPU support required features!");

        physical_device_ = physical_devices[best_grade_index];
        queue_family_indices.Populate(physical_device_, surface_);
    }

    {
        std::unordered_set unique_queue_family_indices = {
            queue_family_indices.graphics_queue_family.value(),
            queue_family_indices.present_queue_family.value(),
        };

        std::vector<vk::DeviceQueueCreateInfo> queue_create_infos;
        queue_create_infos.reserve(unique_queue_family_indices.size());

        float priority = 1.0f;
        for (unsigned unique_queue_family_index : unique_queue_family_indices)
        {
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
            .enabledExtensionCount = static_cast<uint32_t>(device_extensions.size()),
            .ppEnabledExtensionNames = device_extensions.data(),
        };

        device_ = vk::raii::Device{physical_device_, create_info};
        graphics_queue = vk::raii::Queue{device_, queue_family_indices.graphics_queue_family.value(), 0};
        present_queue = vk::raii::Queue{device_, queue_family_indices.present_queue_family.value(), 0};
    }

    // memory allocator
    {
        VmaAllocatorCreateInfo allocator_info = {
            .physicalDevice = *physical_device_,
            .device = *device_,
            .instance = *instance_,
        };
        vma_allocator = vk_types::MemAllocator(&allocator_info);
    }
    // Create command pools
    {
        vk::CommandPoolCreateInfo graphics_command_pool_create_info = {
            .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
            .queueFamilyIndex = queue_family_indices.graphics_queue_family.value(),
        };
        graphics_command_pool = vk::raii::CommandPool{device_, graphics_command_pool_create_info};
    }
}
}
