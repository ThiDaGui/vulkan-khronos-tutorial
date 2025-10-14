import vulkan_hpp;

#include "helloTriangleApplication.hh"

#include <algorithm>
#include <bits/ranges_algo.h>
#include <cstring>
#include <iostream>
#include <set>
#include <sstream>
#include <unordered_map>
#include <vulkan/vulkan_core.h>

//glm
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "vulkanUtils.hh"
#include "config.hh"

//stb_image
#include "stb_image.h"

//tiny_obj_loader
#include "tiny_obj_loader.h"

//imgui
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

void check_vk_result(VkResult err)
{
    if (VK_SUCCESS == err)
        return;
    fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
    if (err < 0)
        abort();
}

VKAPI_ATTR vk::Bool32 VKAPI_CALL
debugCallback(const vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
              const vk::DebugUtilsMessageTypeFlagsEXT messageType,
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

HelloTriangleApplicationCpp::HelloTriangleApplicationCpp()
    : window_{nullptr}
{
    initWindow();
    initVulkan();
}

HelloTriangleApplicationCpp::~HelloTriangleApplicationCpp() {
    cleanup();
}

void HelloTriangleApplicationCpp::run() {
    while (!glfwWindowShouldClose(window_)) {
        glfwPollEvents();
        drawFrame();
    }
    device_.waitIdle();
}

void HelloTriangleApplicationCpp::initWindow() {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    //glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    if (nullptr == (window_ = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr)))
        throw std::runtime_error("failed to create GLFW window!");
    glfwSetFramebufferSizeCallback(window_, framebufferResizeCallback);
    glfwSetWindowUserPointer(window_, this);
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

    createSwapchainImageView();

    createDescriptorSetLayout();

    createGraphicPipeline();

    initImgui();

    createCommandPool();

    createColorBufferResources();
    createDepthBufferResources();

    createTextureImage();
    createTextureImageView();
    createTextureImageSampler();

    loadModel();
    createVertexBuffer();
    createIndexBuffer();

    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();

    createCommandBuffers();

    recordCommandBuffers();

    createSyncObject();
}

void HelloTriangleApplicationCpp::cleanup() {
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window_);
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

#ifndef NDEBUG
    listPhysicalDevices(devices);
#endif

    bool is_suitable = false;
    for (const auto &device: devices) {
        const auto device_properties = device.getProperties();
        const auto device_features = device.getFeatures();

        const auto device_extensions = device.enumerateDeviceExtensionProperties();


        const bool is_vk13_supported = device_properties.apiVersion >= vk::ApiVersion13;
        const bool is_anisotropic_filtering_supported = device_features.samplerAnisotropy;
        RequiredQueueFamilyIndices family_indices{};
        family_indices.Populate(device, surface_);

        bool is_device_extensions_supported = true;

        for (const auto &required_device_extension: required_device_extensions) {
            const bool found =
                    std::ranges::any_of(
                        device_extensions,
                        [required_device_extension](const auto &device_extension) {
                            return std::strcmp(device_extension.extensionName, required_device_extension) == 0;
                        });
            is_device_extensions_supported = is_device_extensions_supported && found;
        }
        is_suitable =
                is_vk13_supported &&
                family_indices.isComplete() &&
                is_device_extensions_supported &&
                is_anisotropic_filtering_supported;
        if (is_suitable) {
            physical_device_ = device;
            queue_family_indices_ = family_indices;
            msaa_samples_ = getUsableSampleCounts();

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

    const std::set unique_queue_indices{queue_family_indices_.graphic_queue.value(), queue_family_indices_.present_queue.value()};
    const auto required_device_extensions = getRequiredDeviceExtensions();
    float queue_priority = 1.0f;

    for (const auto &queue_index: unique_queue_indices) {
        const vk::DeviceQueueCreateInfo device_queue_create_info{
            .queueFamilyIndex = queue_index,
            .queueCount = 1,
            .pQueuePriorities = &queue_priority
        };
        device_queue_create_infos.emplace_back(device_queue_create_info);
    }

    constexpr vk::PhysicalDeviceFeatures physical_device_features = {
        .samplerAnisotropy = true,
    };

    vk::StructureChain<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan11Features, vk::PhysicalDeviceVulkan13Features,
        vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT> feature_chain{
        {.features = physical_device_features},
        {.shaderDrawParameters = true},
        {.synchronization2 = true, .dynamicRendering = true},
        {.extendedDynamicState = true},
    };

    const vk::DeviceCreateInfo device_create_info{
        .pNext = &feature_chain.get<vk::PhysicalDeviceFeatures2>(),
        .queueCreateInfoCount = static_cast<uint32_t>(device_queue_create_infos.size()),
        .pQueueCreateInfos = device_queue_create_infos.data(),
        .enabledExtensionCount = static_cast<uint32_t>(required_device_extensions.size()),
        .ppEnabledExtensionNames = required_device_extensions.data(),
    };

    device_ = vk::raii::Device(physical_device_, device_create_info);
    graphic_queue_ = vk::raii::Queue(device_, queue_family_indices_.graphic_queue.value(), 0);
    present_queue_ = vk::raii::Queue(device_, queue_family_indices_.present_queue.value(), 0);
}

void HelloTriangleApplicationCpp::createSwapChain() {
    const vk::SurfaceCapabilitiesKHR surface_capabilities = physical_device_.getSurfaceCapabilitiesKHR(surface_);

    swapchain_extent_ = chooseExtent2D(surface_capabilities);
    const vk::SurfaceFormatKHR surface_format = chooseSurfaceFormat(physical_device_.getSurfaceFormatsKHR(surface_));
    const vk::PresentModeKHR present_mode = choosePresentMode(physical_device_.getSurfacePresentModesKHR(surface_));

    swapchain_min_image_count = chooseMinImageCount(surface_capabilities);

    vk::SwapchainCreateInfoKHR swapchain_create_info{
        .surface = surface_,
        .minImageCount = swapchain_min_image_count,
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
    } else {
        swapchain_create_info.imageSharingMode = vk::SharingMode::eConcurrent;
        swapchain_create_info.queueFamilyIndexCount = 2;
        swapchain_create_info.pQueueFamilyIndices = queue_family_indices;
    }

    swapchain_ = vk::raii::SwapchainKHR(device_, swapchain_create_info);
    swapchain_images_ = swapchain_.getImages();
    swapchain_image_format_ = surface_format.format;
}

void HelloTriangleApplicationCpp::createSwapchainImageView() {
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

void HelloTriangleApplicationCpp::createColorBufferResources()
{
    createImage(
        swapchain_extent_.width,
        swapchain_extent_.height,
        1,
        msaa_samples_,
        swapchain_image_format_,
        vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransientAttachment,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        color_buffer_,
        color_buffer_memory_
        );

    const vk::ImageViewCreateInfo image_view_create_info = {
        .image = color_buffer_,
        .viewType = vk::ImageViewType::e2D,
        .format = swapchain_image_format_,
        .subresourceRange = {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };

    color_buffer_image_view_ = vk::raii::ImageView(device_, image_view_create_info);

    const auto command_buffer = beginTransientCommandBuffer();
    transitionImageLayout(
        *command_buffer,
        color_buffer_,
        1,
        swapchain_image_format_,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eColorAttachmentOptimal,
        vk::PipelineStageFlagBits2::eTopOfPipe,
        {},
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        vk::AccessFlagBits2::eColorAttachmentWrite
        );
    endTransientCommandBuffer(*command_buffer);
}

void HelloTriangleApplicationCpp::createDepthBufferResources() {
    const vk::Format format = findDepthFormat(physical_device_);
    createImage(
        swapchain_extent_.width,
        swapchain_extent_.height,
        1,
        msaa_samples_,
        format,
        vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eTransientAttachment,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        depth_buffer_,
        depth_buffer_memory_
        );

    const vk::ImageViewCreateInfo image_view_create_info = {
        .image = depth_buffer_,
        .viewType = vk::ImageViewType::e2D,
        .format = format,
        .subresourceRange = {vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1}
    };

    depth_buffer_image_view_ = vk::raii::ImageView(device_, image_view_create_info);

    const auto command_buffer = beginTransientCommandBuffer();
    transitionImageLayout(
        *command_buffer,
        depth_buffer_,
        1,
        format,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eDepthStencilAttachmentOptimal,
        vk::PipelineStageFlagBits2::eTopOfPipe,
        {},
        vk::PipelineStageFlagBits2::eEarlyFragmentTests,
        vk::AccessFlagBits2::eDepthStencilAttachmentRead
        );
    endTransientCommandBuffer(*command_buffer);
}

void HelloTriangleApplicationCpp::createDescriptorSetLayout()
{
    constexpr std::array<vk::DescriptorSetLayoutBinding, 2> layout_bindings{{
        {
            .binding = 0,
            .descriptorType = vk::DescriptorType::eUniformBuffer,
            .descriptorCount = 1,
            .stageFlags = vk::ShaderStageFlagBits::eVertex,
            .pImmutableSamplers = nullptr
        },
        {
                .binding = 1,
                .descriptorType = vk::DescriptorType::eCombinedImageSampler,
                .descriptorCount = 1,
                .stageFlags = vk::ShaderStageFlagBits::eFragment,
        }
    }};

    const vk::DescriptorSetLayoutCreateInfo layout_create_info = {
        .bindingCount = layout_bindings.size(),
        .pBindings = layout_bindings.data(),
    };

    descriptor_set_layout_ = vk::raii::DescriptorSetLayout(device_, layout_create_info);
}

void HelloTriangleApplicationCpp::createGraphicPipeline() {
    vk::raii::ShaderModule shader_module = createShaderModule(shaderPath / "triangle_slang.spv");

    const vk::PipelineShaderStageCreateInfo vertex_shader_stage_info{
        .stage = vk::ShaderStageFlagBits::eVertex,
        .module = shader_module,
        .pName = "vertMain",
    };

    const vk::PipelineShaderStageCreateInfo fragment_shader_stage_info{
        .stage = vk::ShaderStageFlagBits::eFragment,
        .module = shader_module,
        .pName = "fragMain",
    };

    const vk::PipelineShaderStageCreateInfo shader_stage_create_infos[2]{
        vertex_shader_stage_info,
        fragment_shader_stage_info,
    };

    const std::vector<vk::DynamicState> dynamic_states{
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor,
    };

    const vk::PipelineDynamicStateCreateInfo dynamic_state_create_info{
        .dynamicStateCount = static_cast<std::uint32_t>(dynamic_states.size()),
        .pDynamicStates = dynamic_states.data(),
    };

    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();

    const vk::PipelineVertexInputStateCreateInfo vertex_input_state_create_info{
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &bindingDescription,
        .vertexAttributeDescriptionCount = attributeDescriptions.size(),
        .pVertexAttributeDescriptions = attributeDescriptions.data(),
    };

    constexpr vk::PipelineInputAssemblyStateCreateInfo input_assembly_state_create_info{
        .topology = vk::PrimitiveTopology::eTriangleList,
    };

    constexpr vk::PipelineViewportStateCreateInfo viewport_state_create_info{
        .viewportCount = 1,
        .scissorCount = 1
    };

    constexpr vk::PipelineRasterizationStateCreateInfo rasterization_state_create_info{
        .depthClampEnable = vk::False,
        .rasterizerDiscardEnable = vk::False,
        .polygonMode = vk::PolygonMode::eFill,
        .cullMode = vk::CullModeFlagBits::eBack,
        .frontFace = vk::FrontFace::eCounterClockwise,
        .depthBiasEnable = vk::False,
        .lineWidth = 1.0,
    };

    const vk::PipelineMultisampleStateCreateInfo multisample_state_create_info{
        .rasterizationSamples = msaa_samples_,
        .sampleShadingEnable = vk::False,
    };

    constexpr vk::PipelineDepthStencilStateCreateInfo depth_stencil_state_create_info = {
        .depthTestEnable = vk::True,
        .depthWriteEnable = vk::True,
        .depthCompareOp = vk::CompareOp::eLess
    };

    constexpr vk::PipelineColorBlendAttachmentState color_blend_attachment_state{
        .blendEnable = vk::False,
        .colorWriteMask = vk::ColorComponentFlagBits::eR |
                          vk::ColorComponentFlagBits::eG |
                          vk::ColorComponentFlagBits::eB |
                          vk::ColorComponentFlagBits::eA,
    };

    const vk::PipelineColorBlendStateCreateInfo color_blend_state_create_info {
        .logicOpEnable = vk::False,
        .logicOp = vk::LogicOp::eCopy,
        .attachmentCount = 1,
        .pAttachments = &color_blend_attachment_state,
    };

    vk::PipelineLayoutCreateInfo pipeline_layout_create_info {
        .setLayoutCount = 1,
        .pSetLayouts = &*descriptor_set_layout_,
        .pushConstantRangeCount = 0,
    };

    pipeline_layout_ = vk::raii::PipelineLayout{ device_, pipeline_layout_create_info };

    vk::PipelineRenderingCreateInfo pipeline_rendering_create_info = {
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &swapchain_image_format_,
        .depthAttachmentFormat = findDepthFormat(physical_device_),
    };

    const vk::GraphicsPipelineCreateInfo pipeline_create_info {
        .pNext = pipeline_rendering_create_info,
        .stageCount = 2,
        .pStages = shader_stage_create_infos,
        .pVertexInputState = &vertex_input_state_create_info,
        .pInputAssemblyState = &input_assembly_state_create_info,
        .pViewportState = &viewport_state_create_info,
        .pRasterizationState = &rasterization_state_create_info,
        .pMultisampleState = &multisample_state_create_info,
        .pDepthStencilState = &depth_stencil_state_create_info,
        .pColorBlendState = &color_blend_state_create_info,
        .pDynamicState = &dynamic_state_create_info,
        .layout = pipeline_layout_,
        .renderPass = nullptr,
    };

    graphic_pipeline_ = vk::raii::Pipeline{device_, nullptr, pipeline_create_info};
}

void HelloTriangleApplicationCpp::createCommandPool() {
    const vk::CommandPoolCreateInfo command_pool_create_info = {
        .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        .queueFamilyIndex = queue_family_indices_.graphic_queue.value(),
    };

    command_pool_ = vk::raii::CommandPool(device_, command_pool_create_info);

    const vk::CommandPoolCreateInfo transient_command_pool_create_info{
        .flags = vk::CommandPoolCreateFlagBits::eTransient,
        .queueFamilyIndex = queue_family_indices_.graphic_queue.value(),
    };

    transient_command_pool_ = vk::raii::CommandPool(device_, transient_command_pool_create_info);
}

void HelloTriangleApplicationCpp::createTextureImage() {
    int texture_width, texture_height, texture_channels;
    stbi_set_flip_vertically_on_load(true);
    stbi_uc *pixels = stbi_load((texturePath / "viking_room.png").c_str(), &texture_width, &texture_height, &texture_channels, 4);
    const vk::DeviceSize image_size = texture_width *texture_height * 4;
    stbi_set_flip_vertically_on_load(false);

    if (!pixels)
        throw std::runtime_error("Failed to load texture image!");

    texture_image_mip_levels_ = static_cast<uint32_t>(std::floor(std::log2(std::max(texture_width, texture_height))));

    vk::raii::Buffer image_buffer{nullptr};
    vk::raii::DeviceMemory image_buffer_memory{nullptr};
    createBuffer(
        image_size,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
        image_buffer,
        image_buffer_memory);

    void *data = image_buffer_memory.mapMemory(0, vk::WholeSize);
    memcpy(data, pixels, image_size);
    stbi_image_free(pixels);

    createImage(
        texture_width,
        texture_height,
        texture_image_mip_levels_,
        vk::SampleCountFlagBits::e1,
        vk::Format::eR8G8B8A8Srgb,
        vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eTransferSrc |
        vk::ImageUsageFlagBits::eTransferDst|
        vk::ImageUsageFlagBits::eSampled,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        texture_image_,
        texture_image_memory_
        );

    const auto command_buffer = beginTransientCommandBuffer();
    transitionImageLayout(
        *command_buffer,
        texture_image_,
        texture_image_mip_levels_,
        vk::Format::eR8G8B8A8Srgb,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eTransferDstOptimal,
        vk::PipelineStageFlagBits2::eTopOfPipe,
        {},
        vk::PipelineStageFlagBits2::eTransfer,
        vk::AccessFlagBits2::eTransferWrite
        );
    const vk::BufferImageCopy buffer_image_copy = {
        .imageSubresource = {vk::ImageAspectFlagBits::eColor, 0, 0, 1},
        .imageExtent = {static_cast<uint32_t>(texture_width), static_cast<uint32_t>(texture_height), 1},
    };
    command_buffer->copyBufferToImage(image_buffer, texture_image_, vk::ImageLayout::eTransferDstOptimal, buffer_image_copy);

    vk::FormatProperties format_properties = physical_device_.getFormatProperties(vk::Format::eR8G8B8A8Srgb);
    if (!(format_properties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear))
        throw std::runtime_error("texture image format does not support linear blitting!");

    generateMips(*command_buffer, texture_image_, texture_width, texture_height, texture_image_mip_levels_);

    endTransientCommandBuffer(*command_buffer);
}

void HelloTriangleApplicationCpp::createTextureImageView()
{
    const vk::ImageViewCreateInfo texture_image_view_create_info = {
        .image = texture_image_,
        .viewType = vk::ImageViewType::e2D,
        .format = vk::Format::eR8G8B8A8Srgb,
        .subresourceRange = {
            vk::ImageAspectFlagBits::eColor,
            0,
            texture_image_mip_levels_,
            0,
            1
        }
    };

    texture_image_view_ = vk::raii::ImageView(device_, texture_image_view_create_info);
}

void HelloTriangleApplicationCpp::createTextureImageSampler()
{
    const vk::PhysicalDeviceProperties properties = physical_device_.getProperties();

    const vk::SamplerCreateInfo sampler_create_info ={
        .magFilter = vk::Filter::eLinear,
        .minFilter = vk::Filter::eLinear,
        .mipmapMode = vk::SamplerMipmapMode::eLinear,
        .addressModeU = vk::SamplerAddressMode::eRepeat,
        .addressModeV = vk::SamplerAddressMode::eRepeat,
        .mipLodBias = 0.0f,
        .anisotropyEnable = true,
        .maxAnisotropy = properties.limits.maxSamplerAnisotropy,
    };

    texture_image_sampler_ = vk::raii::Sampler(device_, sampler_create_info);
}

void HelloTriangleApplicationCpp::loadModel()
{
    tinyobj::attrib_t attrib{};
    std::vector<tinyobj::shape_t> shapes{};
    std::vector<tinyobj::material_t> materials{};
    std::string warn{}, err{};

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, (modelPath / "viking_room.obj").c_str()))
        throw std::runtime_error(warn + err);

    std::unordered_map<Vertex, uint32_t> unique_vertices{};

    for (const auto &shape : shapes) {
        for (const auto &index : shape.mesh.indices) {
            Vertex vertex{};

            vertex.position = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };
            vertex.tex_coordinates = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                attrib.texcoords[2 * index.texcoord_index + 1]
            };

            if (!unique_vertices.contains(vertex)) {
                unique_vertices[vertex] = static_cast<uint32_t>(unique_vertices.size());
                vertices_.push_back(vertex);
            }
            indices_.push_back(unique_vertices[vertex]);
        }
    }
}

void HelloTriangleApplicationCpp::createVertexBuffer()
{
    const vk::DeviceSize vertex_buffer_size = vertices_.size() * sizeof(Vertex);
    vk::raii::Buffer staging_buffer{nullptr};
    vk::raii::DeviceMemory staging_buffer_memory{nullptr};

    createBuffer(
        vertex_buffer_size,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible,
        staging_buffer,
        staging_buffer_memory
        );

    void * const data = staging_buffer_memory.mapMemory(0, vertex_buffer_size);
    memcpy(data, vertices_.data(), vertex_buffer_size);
    staging_buffer_memory.unmapMemory();

    createBuffer(
        vertex_buffer_size,

        vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        vertex_buffer_,
        vertex_buffer_memory_
        );

    copyBuffer(staging_buffer, vertex_buffer_, 0, 0, vertex_buffer_size);
}

void HelloTriangleApplicationCpp::createIndexBuffer()
{
    const vk::DeviceSize index_buffer_size = indices_.size() * sizeof(indices_[0]);
    vk::raii::Buffer staging_buffer{nullptr};
    vk::raii::DeviceMemory staging_buffer_memory{nullptr};

    createBuffer(
        index_buffer_size,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
        staging_buffer,
        staging_buffer_memory
        );

    void * const data = staging_buffer_memory.mapMemory(0, index_buffer_size);
    memcpy(data, indices_.data(), index_buffer_size);
    staging_buffer_memory.unmapMemory();

    createBuffer(
        index_buffer_size,
        vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        index_buffer_,
        index_buffer_memory_
        );

    copyBuffer(staging_buffer, index_buffer_, 0, 0, index_buffer_size);
}

void HelloTriangleApplicationCpp::createUniformBuffers()
{
    mvp_uniform_buffers_.clear();
    mvp_uniform_buffers_memory_.clear();
    mvp_uniform_buffers_mapped_.clear();

    for (std::size_t i = 0; i < swapchain_images_.size(); i++) {
        constexpr vk::DeviceSize buffer_size = sizeof(MVPUniformBuffer);

        vk::raii::Buffer buffer{nullptr};
        vk::raii::DeviceMemory buffer_memory{nullptr};

        createBuffer(
            buffer_size,
            vk::BufferUsageFlagBits::eUniformBuffer,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
            buffer,
            buffer_memory);

        mvp_uniform_buffers_.emplace_back(std::move(buffer));
        mvp_uniform_buffers_memory_.emplace_back(std::move(buffer_memory));
        mvp_uniform_buffers_mapped_.emplace_back(mvp_uniform_buffers_memory_[i].mapMemory(0, buffer_size));
    }
}

void HelloTriangleApplicationCpp::createDescriptorPool()
{
    const std::array<vk::DescriptorPoolSize, 2> pool_sizes = {{
        {
            .type = vk::DescriptorType::eUniformBuffer,
            .descriptorCount = static_cast<uint32_t>(swapchain_images_.size()),
        },
        {
            .type = vk::DescriptorType::eCombinedImageSampler,
            .descriptorCount = static_cast<uint32_t>(swapchain_images_.size()),
        }
    }};

    const vk::DescriptorPoolCreateInfo descriptor_pool_create_info = {
        .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
        .maxSets = static_cast<uint32_t>(swapchain_images_.size()),
        .poolSizeCount = static_cast<uint32_t>(pool_sizes.size()),
        .pPoolSizes = pool_sizes.data(),
    };

    descriptor_pool_ = vk::raii::DescriptorPool(device_, descriptor_pool_create_info);
}

void HelloTriangleApplicationCpp::createDescriptorSets() {
    std::vector<vk::DescriptorSetLayout> layouts{swapchain_images_.size(), descriptor_set_layout_};
    vk::DescriptorSetAllocateInfo allocate_info = {
        .descriptorPool = descriptor_pool_,
        .descriptorSetCount = static_cast<uint32_t>(layouts.size()),
        .pSetLayouts = layouts.data()
    };

    descriptor_sets_ = device_.allocateDescriptorSets(allocate_info);

    for (size_t i = 0; i < swapchain_images_.size(); i++) {
        vk::DescriptorBufferInfo buffer_info = {
            .buffer = mvp_uniform_buffers_[i],
            .offset = 0,
            .range = sizeof(MVPUniformBuffer),
        };
        vk::DescriptorImageInfo texture_image_info = {
            .sampler = texture_image_sampler_,
            .imageView = texture_image_view_,
            .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
        };


        std::array<vk::WriteDescriptorSet, 2> write_descriptor_set = {{
            {
                .dstSet = descriptor_sets_[i],
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = vk::DescriptorType::eUniformBuffer,
                .pBufferInfo = &buffer_info,
            },
            {
                .dstSet = descriptor_sets_[i],
                .dstBinding = 1,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = vk::DescriptorType::eCombinedImageSampler,
                .pImageInfo = &texture_image_info,
            }
        }};

        device_.updateDescriptorSets(write_descriptor_set, {});
    }
}

void HelloTriangleApplicationCpp::createCommandBuffers() {
    const vk::CommandBufferAllocateInfo command_buffer_allocate_info {
        .commandPool = command_pool_,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = static_cast<uint32_t>(swapchain_images_.size()),
    };

    command_buffers_ = vk::raii::CommandBuffers{device_, command_buffer_allocate_info};
    imgui_command_buffers_ = vk::raii::CommandBuffers(device_, command_buffer_allocate_info);
}

void HelloTriangleApplicationCpp::recordCommandBuffers() const {
    for (size_t i = 0; i < swapchain_images_.size(); i++) {
        recordCommandBuffer(i);
    }
}

void HelloTriangleApplicationCpp::recordCommandBuffer(const uint32_t image_index) const {
    command_buffers_[image_index].begin({});

    // AS I understand, with previous iteration of the Vulkan Tutorial
    // this transition was handled by the RenderPass
    transitionImageLayout(
        command_buffers_[image_index],
        swapchain_images_[image_index],
        1,
        swapchain_image_format_,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eColorAttachmentOptimal,
        vk::PipelineStageFlagBits2::eTopOfPipe,
        {},
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        vk::AccessFlagBits2::eColorAttachmentWrite
        );

    constexpr vk::ClearValue clear_color = vk::ClearColorValue{0.0f, 0.0f, 0.0f, 1.0f};
    vk::RenderingAttachmentInfo attachment_info = {
        .imageView = swapchain_image_views_[image_index],
        .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .clearValue = clear_color,
    };
    if (msaa_samples_ != vk::SampleCountFlagBits::e1) {
        attachment_info.imageView = color_buffer_image_view_;
        attachment_info.resolveMode = vk::ResolveModeFlagBits::eAverage;
        attachment_info.resolveImageView = swapchain_image_views_[image_index];
        attachment_info.resolveImageLayout = vk::ImageLayout::eColorAttachmentOptimal;
        attachment_info.storeOp = vk::AttachmentStoreOp::eDontCare;
    }

    constexpr vk::ClearValue clear_depth = vk::ClearDepthStencilValue{1.0f, 0};
    vk::RenderingAttachmentInfo depth_attachment_info = {
        .imageView = depth_buffer_image_view_,
        .imageLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eDontCare,
        .clearValue = clear_depth,
    };

    const vk::RenderingInfo rendering_info = {
        .renderArea = {.offset = {0, 0}, .extent = swapchain_extent_},
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &attachment_info,
        .pDepthAttachment = &depth_attachment_info,
    };

    command_buffers_[image_index].beginRendering(rendering_info);

    command_buffers_[image_index].bindPipeline(vk::PipelineBindPoint::eGraphics, graphic_pipeline_);

    {
        const vk::Viewport viewport = {
            0.0f,
            0.0f,
            static_cast<float>(swapchain_extent_.width),
            static_cast<float>(swapchain_extent_.height),
            0.0f,
            1.0f
        };

        const vk::Rect2D scissor{{0, 0}, swapchain_extent_};

        command_buffers_[image_index].setViewport(0, viewport);
        command_buffers_[image_index].setScissor(0, scissor);
    }

    command_buffers_[image_index].bindVertexBuffers(0, *vertex_buffer_, {0});
    command_buffers_[image_index].bindIndexBuffer(index_buffer_, 0, vk::IndexType::eUint32);

    command_buffers_[image_index].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline_layout_, 0, *descriptor_sets_[image_index], nullptr);
    command_buffers_[image_index].drawIndexed(indices_.size(), 1, 0, 0, 0);

    command_buffers_[image_index].endRendering();

    command_buffers_[image_index].end();
}

void HelloTriangleApplicationCpp::recordImguiCommandBuffer(
    const uint32_t image_index) const
{
    imgui_command_buffers_[image_index].begin({});

    transitionImageLayout(
        imgui_command_buffers_[image_index],
        swapchain_images_[image_index],
        1,
        swapchain_image_format_,
        vk::ImageLayout::eColorAttachmentOptimal,
        vk::ImageLayout::eColorAttachmentOptimal,
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        vk::AccessFlagBits2::eColorAttachmentWrite,
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        vk::AccessFlagBits2::eColorAttachmentWrite
        );

    const vk::RenderingAttachmentInfo attachment_info = {
        .imageView = swapchain_image_views_[image_index],
        .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .loadOp = vk::AttachmentLoadOp::eLoad,
        .storeOp = vk::AttachmentStoreOp::eStore,
    };

    const vk::RenderingInfo rendering_info = {
        .renderArea = {.offset = {0, 0}, .extent = swapchain_extent_},
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &attachment_info,
    };

    imgui_command_buffers_[image_index].beginRendering(rendering_info);

    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), *imgui_command_buffers_[image_index]);

    imgui_command_buffers_[image_index].endRendering();
    transitionImageLayout(
        imgui_command_buffers_[image_index],
        swapchain_images_[image_index],
        1,
        swapchain_image_format_,
        vk::ImageLayout::eColorAttachmentOptimal,
        vk::ImageLayout::ePresentSrcKHR,
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        vk::AccessFlagBits2::eColorAttachmentWrite,
        vk::PipelineStageFlagBits2::eBottomOfPipe,
        {}
        );

    imgui_command_buffers_[image_index].end();
}

void HelloTriangleApplicationCpp::initImgui() const
{
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui_ImplGlfw_InitForVulkan(&*window_, true);

    vk::PipelineRenderingCreateInfo pipeline_rendering_create_info = {
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &swapchain_image_format_,
    };

    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.ApiVersion = vk::ApiVersion14;
    init_info.Instance = *instance_;
    init_info.PhysicalDevice = *physical_device_;
    init_info.Device = *device_;
    init_info.QueueFamily = queue_family_indices_.graphic_queue.value();
    init_info.Queue = *graphic_queue_;
    init_info.DescriptorPoolSize = IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE;
    init_info.MinImageCount = swapchain_min_image_count;
    init_info.ImageCount = swapchain_images_.size();
    init_info.UseDynamicRendering = true;
    init_info.PipelineInfoMain.PipelineRenderingCreateInfo = pipeline_rendering_create_info;
    init_info.PipelineInfoMain.MSAASamples = static_cast<VkSampleCountFlagBits>(vk::SampleCountFlagBits::e1);
    init_info.CheckVkResultFn = &check_vk_result;

    ImGui_ImplVulkan_Init(&init_info);

}

void HelloTriangleApplicationCpp::createSyncObject() {
    present_complete_semaphores_.clear();
    render_finished_semaphores_.clear();
    in_flight_fences_.clear();

    for (size_t i = 0; i < swapchain_images_.size(); i++) {
        present_complete_semaphores_.emplace_back(device_, vk::SemaphoreCreateInfo{});
        render_finished_semaphores_.emplace_back(device_, vk::SemaphoreCreateInfo{});
    }
    for (std::size_t i = 0; i < MAX_FRAME_IN_FLIGHT; i++) {
        in_flight_fences_.emplace_back(device_, vk::FenceCreateInfo{.flags = vk::FenceCreateFlagBits::eSignaled});
    }
}

void HelloTriangleApplicationCpp::cleanupSwapChain() {
    swapchain_image_views_.clear();
    swapchain_.clear();
}

void HelloTriangleApplicationCpp::recreateSwapChain() {
    int width, height;
    glfwGetWindowSize(window_, &width, &height);
    while (width == 0 && height == 0) {
        glfwWaitEvents();
        glfwGetWindowSize(window_, &width, &height);
    }
    device_.waitIdle();

    cleanupSwapChain();

    createSwapChain();
    createSwapchainImageView();
    createColorBufferResources();
    createDepthBufferResources();
    recordCommandBuffers();
}

void HelloTriangleApplicationCpp::drawFrame() {
    static uint32_t fence_index = 0;
    static uint32_t present_semaphore_index = 0;

    while (vk::Result::eTimeout == device_.waitForFences(*in_flight_fences_[fence_index], vk::True, std::numeric_limits<uint64_t>::max()))
    {}
    device_.resetFences(*in_flight_fences_[fence_index]);

    auto [result, image_index] =
        swapchain_.acquireNextImage(std::numeric_limits<uint64_t>::max(), present_complete_semaphores_[present_semaphore_index],nullptr);
    if (result == vk::Result::eErrorOutOfDateKHR) {
        recreateSwapChain();
        return;
    }
    if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
        throw std::runtime_error("Failed to acquire swap chain image !");
    image_index_ = image_index;

    Update();
    std::array<vk::CommandBuffer, 2> command_buffers {
        command_buffers_[image_index_],
        imgui_command_buffers_[image_index_],
    };

    constexpr vk::PipelineStageFlags wait_destination_stage_mask{vk::PipelineStageFlagBits::eColorAttachmentOutput};
    const vk::SubmitInfo submit_info{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &*present_complete_semaphores_[present_semaphore_index],
        .pWaitDstStageMask = &wait_destination_stage_mask,
        .commandBufferCount = command_buffers.size(),
        .pCommandBuffers = command_buffers.data(),
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &*render_finished_semaphores_[image_index_],
    };

    graphic_queue_.submit(submit_info, *in_flight_fences_[fence_index]);

    const vk::PresentInfoKHR present_info{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &*render_finished_semaphores_[image_index_],
        .swapchainCount = 1,
        .pSwapchains = &*swapchain_,
        .pImageIndices = &image_index_,
    };

    result = present_queue_.presentKHR(present_info);
    if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || framebufferResized) {
        framebufferResized = false;
        recreateSwapChain();
    }
    else if (result != vk::Result::eSuccess)
        throw std::runtime_error("Failed to present swapchain image !");

    present_semaphore_index = (present_semaphore_index + 1) % present_complete_semaphores_.size();
    fence_index = (fence_index + 1) % MAX_FRAME_IN_FLIGHT;
}


void HelloTriangleApplicationCpp::Update() const
{
    UpdateMVPUniformBuffer();
    UpdateImGui();
}

void HelloTriangleApplicationCpp::UpdateMVPUniformBuffer() const
{
    static auto start_time = std::chrono::high_resolution_clock::now();

    const auto current_time = std::chrono::high_resolution_clock::now();
    const float time = std::chrono::duration<float, std::chrono::seconds::period>(current_time - start_time).count();
    MVPUniformBuffer mvp{};
    mvp.model = glm::rotate(glm::mat4{1.0f}, glm::sin(time) * glm::radians(30.0f), glm::vec3{0.0f, 0.0f, 1.0f});
    mvp.view = glm::lookAt(glm::vec3{2.0f, 2.0f, 2.25f}, glm::vec3{0.0f, 0.0f, 0.25f}, glm::vec3{0.0f, 0.0f, 1.0f});
    mvp.proj = glm::perspective(glm::radians(30.0f), static_cast<float>(swapchain_extent_.width) / static_cast<float>(swapchain_extent_.height), 0.1f, 10.0f);
    mvp.proj[1][1] *= -1;

    memcpy(mvp_uniform_buffers_mapped_[image_index_], &mvp, sizeof(mvp));
}

void HelloTriangleApplicationCpp::UpdateImGui() const
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    static bool show_demo_window = true;
    if (show_demo_window)
        ImGui::ShowDemoWindow(&show_demo_window);

    ImGui::Render();
    recordImguiCommandBuffer(image_index_);

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


vk::Extent2D HelloTriangleApplicationCpp::chooseExtent2D(const vk::SurfaceCapabilitiesKHR &surface_capabilities) const {
    if (surface_capabilities.currentExtent.width != std::numeric_limits<std::uint32_t>::max())
        return surface_capabilities.currentExtent;

    std::int32_t width, height;
    glfwGetFramebufferSize(window_, &width, &height);

    return {
        std::clamp<std::uint32_t>(width, surface_capabilities.minImageExtent.width,
                                  surface_capabilities.maxImageExtent.width),
        std::clamp<std::uint32_t>(height, surface_capabilities.minImageExtent.height,
                                  surface_capabilities.maxImageExtent.height)
    };
}

vk::raii::ShaderModule HelloTriangleApplicationCpp::createShaderModule(const std::filesystem::path &shader_path) const {
    std::vector buffer{readShader(shader_path)};

    const vk::ShaderModuleCreateInfo shader_module_create_info{
        .codeSize = buffer.size() * sizeof(char),
        .pCode = reinterpret_cast<uint32_t *>(buffer.data()),
    };

    vk::raii::ShaderModule shader_module{device_, shader_module_create_info};

    return shader_module;
}

void HelloTriangleApplicationCpp::framebufferResizeCallback(GLFWwindow *window, int width, int height) {
    const auto app = static_cast<HelloTriangleApplicationCpp *>(glfwGetWindowUserPointer(window));
    app->framebufferResized = true;
}

void HelloTriangleApplicationCpp::createBuffer(
    const vk::DeviceSize buffer_size,
    const vk::BufferUsageFlags buffer_usage,
    const vk::MemoryPropertyFlags memory_properties,
    vk::raii::Buffer &buffer,
    vk::raii::DeviceMemory &buffer_memory) const
{
    const vk::BufferCreateInfo buffer_create_info{
        .size = buffer_size,
        .usage = buffer_usage,
        .sharingMode = vk::SharingMode::eExclusive
    };

    buffer = vk::raii::Buffer(device_, buffer_create_info);

    const vk::MemoryRequirements memory_requirements = buffer.getMemoryRequirements();
    const vk::PhysicalDeviceMemoryProperties physical_device_memory_properties =
        physical_device_.getMemoryProperties();

    const std::uint32_t memory_type_index =
        findMemoryTypeIndex(physical_device_memory_properties,
            memory_requirements.memoryTypeBits,
            memory_properties);

    const vk::MemoryAllocateInfo allocate_info{
        .allocationSize = memory_requirements.size,
        .memoryTypeIndex = memory_type_index,
    };

    buffer_memory = vk::raii::DeviceMemory(device_, allocate_info);
    buffer.bindMemory(buffer_memory, 0);
}

void HelloTriangleApplicationCpp::copyBuffer(
    const vk::raii::Buffer &src,
    const vk::raii::Buffer &dst,
    const vk::DeviceSize src_offset,
    const vk::DeviceSize dst_offset,
    const vk::DeviceSize size) const
{
    const vk::CommandBufferAllocateInfo copy_command_buffer_alloc_info = {
        .commandPool = transient_command_pool_,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1,
    };

    const vk::raii::CommandBuffer copy_command_buffer =
        std::move(device_.allocateCommandBuffers(copy_command_buffer_alloc_info).front());

    copy_command_buffer.begin({.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
    copy_command_buffer.copyBuffer(
        src,
        dst,
        vk::BufferCopy(src_offset, dst_offset, size));
    copy_command_buffer.end();

    graphic_queue_.submit(
        vk::SubmitInfo{.commandBufferCount = 1, .pCommandBuffers = &*copy_command_buffer},
        nullptr);
    graphic_queue_.waitIdle();
}

void HelloTriangleApplicationCpp::createImage(
    const uint32_t width,
    const uint32_t height,
    const uint32_t mip_levels,
    const vk::SampleCountFlagBits samples,
    const vk::Format format,
    const vk::ImageTiling tiling,
    const vk::ImageUsageFlags image_usage_flags,
    const vk::MemoryPropertyFlags memory_property_flags,
    vk::raii::Image &image,
    vk::raii::DeviceMemory &image_memory) const
{
    const vk::ImageCreateInfo image_create_info = {
        .imageType = vk::ImageType::e2D,
        .format = format,
        .extent = {width, height, 1},
        .mipLevels = mip_levels,
        .arrayLayers = 1,
        .samples = samples,
        .tiling = tiling,
        .usage = image_usage_flags,
        .sharingMode = vk::SharingMode::eExclusive,
        .initialLayout = vk::ImageLayout::eUndefined,
    };
    image = vk::raii::Image(device_, image_create_info);

    const vk::MemoryRequirements memory_requirements = image.getMemoryRequirements();
    const vk::PhysicalDeviceMemoryProperties physical_device_memory_properties =
        physical_device_.getMemoryProperties();

    const std::uint32_t memory_type_index =
        findMemoryTypeIndex(physical_device_memory_properties,
            memory_requirements.memoryTypeBits,
            memory_property_flags);

    const vk::MemoryAllocateInfo allocate_info =
    {
        .allocationSize = memory_requirements.size,
        .memoryTypeIndex = memory_type_index,
    };

    image_memory = vk::raii::DeviceMemory(device_, allocate_info);
    image.bindMemory(image_memory, 0);
}

vk::SampleCountFlagBits
HelloTriangleApplicationCpp::getUsableSampleCounts() const
{
    const vk::PhysicalDeviceProperties physical_device_properties = physical_device_.getProperties();

    const vk::SampleCountFlags count =
        physical_device_properties.limits.framebufferColorSampleCounts &
            physical_device_properties.limits.framebufferDepthSampleCounts;

    auto sample_count = vk::SampleCountFlagBits::e1;

    if (count & vk::SampleCountFlagBits::e64) {
        sample_count = vk::SampleCountFlagBits::e64;
    }
    else if (count & vk::SampleCountFlagBits::e32) {
        sample_count = vk::SampleCountFlagBits::e32;
    }
    else if (count & vk::SampleCountFlagBits::e16) {
        sample_count = vk::SampleCountFlagBits::e16;
    }
    else if (count & vk::SampleCountFlagBits::e8) {
        sample_count = vk::SampleCountFlagBits::e8;
    }
    else if (count & vk::SampleCountFlagBits::e4) {
        sample_count = vk::SampleCountFlagBits::e4;
    }
    else if (count & vk::SampleCountFlagBits::e2) {
        sample_count = vk::SampleCountFlagBits::e2;
    }

#ifndef NDEBUG
    listUsableSampleCounts(count, sample_count);
#endif
    return sample_count;
}

std::unique_ptr<vk::raii::CommandBuffer>
HelloTriangleApplicationCpp::beginTransientCommandBuffer() const

{
    const vk::CommandBufferAllocateInfo allocate_info = {
        .commandPool = transient_command_pool_,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1
    };

    auto command_buffer = std::make_unique<vk::raii::CommandBuffer>(std::move(device_.allocateCommandBuffers(allocate_info).front()));
    command_buffer->begin({.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit});

    return command_buffer;
}

void HelloTriangleApplicationCpp::endTransientCommandBuffer(
    const vk::raii::CommandBuffer &command_buffer) const
{
    command_buffer.end();

    const vk::SubmitInfo submit_info = {
        .commandBufferCount = 1,
        .pCommandBuffers = &*command_buffer,
    };
    graphic_queue_.submit(submit_info);
    graphic_queue_.waitIdle();
}
