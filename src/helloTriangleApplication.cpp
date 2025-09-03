import vulkan_hpp;

#include "helloTriangleApplication.hh"

#include <algorithm>
#include <bits/ranges_algo.h>
#include <cstring>
#include <iostream>
#include <set>
#include <sstream>

#include "vulkanUtils.hh"
#include "config.hh"
#include "vulkan/vulkan_raii.hpp"

VKAPI_ATTR vk::Bool32 VKAPI_CALL
debugCallback(const vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
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

    createGraphicPipeline();

    createCommandPool();

    createCommandBuffers();

    recordCommandBuffers();

    createSyncObject();
}

void HelloTriangleApplicationCpp::cleanup() {
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
                        [required_device_extension](const auto &device_extension) {
                            return std::strcmp(device_extension.extensionName, required_device_extension) == 0;
                        });
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

    for (const auto &queue_index: unique_queue_indices) {
        const vk::DeviceQueueCreateInfo device_queue_create_info{
            .queueFamilyIndex = queue_index,
            .queueCount = 1,
            .pQueuePriorities = &queue_priority
        };
        device_queue_create_infos.emplace_back(device_queue_create_info);
    }

    vk::StructureChain<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan11Features, vk::PhysicalDeviceVulkan13Features,
        vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT> feature_chain{
        {},
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

    constexpr vk::PipelineVertexInputStateCreateInfo vertex_input_state_create_info{};

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

    constexpr vk::PipelineMultisampleStateCreateInfo multisample_state_create_info{
        .rasterizationSamples = vk::SampleCountFlagBits::e1,
        .sampleShadingEnable = vk::False,
    };

    static constexpr vk::PipelineColorBlendAttachmentState color_blend_attachment_state{
        .blendEnable = vk::False,
        .colorWriteMask = vk::ColorComponentFlagBits::eR |
                          vk::ColorComponentFlagBits::eG |
                          vk::ColorComponentFlagBits::eB |
                          vk::ColorComponentFlagBits::eA,
    };

    constexpr vk::PipelineColorBlendStateCreateInfo color_blend_state_create_info {
        .logicOpEnable = vk::False,
        .logicOp = vk::LogicOp::eCopy,
        .attachmentCount = 1,
        .pAttachments = &color_blend_attachment_state,
    };

    vk::PipelineLayoutCreateInfo pipeline_layout_create_info {
        .setLayoutCount = 0,
        .pushConstantRangeCount = 0,
    };

    pipeline_layout_ = vk::raii::PipelineLayout{ device_, pipeline_layout_create_info };

    const vk::PipelineRenderingCreateInfo pipeline_rendering_create_info {
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &swapchain_image_format_,
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
        .pDepthStencilState = nullptr,
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
}

void HelloTriangleApplicationCpp::createCommandBuffers() {
    const vk::CommandBufferAllocateInfo command_buffer_allocate_info {
        .commandPool = command_pool_,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = static_cast<uint32_t>(swapchain_images_.size()),
    };

    command_buffer_ = vk::raii::CommandBuffers{device_, command_buffer_allocate_info};
}

void HelloTriangleApplicationCpp::recordCommandBuffers() const {
    for (size_t i = 0; i < swapchain_images_.size(); i++) {
        recordCommandBuffer(i);
    }
}

void HelloTriangleApplicationCpp::recordCommandBuffer(const uint32_t image_index) const {
    command_buffer_[image_index].begin({});

    transitionImageLayout(
        command_buffer_[image_index],
        swapchain_images_[image_index],
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eColorAttachmentOptimal,
        {},
        vk::AccessFlagBits2::eColorAttachmentWrite,
        vk::PipelineStageFlagBits2::eTopOfPipe,
        vk::PipelineStageFlagBits2::eColorAttachmentOutput
    );

    constexpr vk::ClearValue clear_color = vk::ClearColorValue{0.0f, 0.0f, 0.0f, 1.0f};
    vk::RenderingAttachmentInfo attachment_info = {
        .imageView = swapchain_image_views_[image_index],
        .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .clearValue = clear_color,
    };

    const vk::RenderingInfo rendering_info = {
        .renderArea = {.offset = {0, 0}, .extent = swapchain_extent_},
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &attachment_info,
    };

    command_buffer_[image_index].beginRendering(rendering_info);

    command_buffer_[image_index].bindPipeline(vk::PipelineBindPoint::eGraphics, graphic_pipeline_);

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

        command_buffer_[image_index].setViewport(0, viewport);
        command_buffer_[image_index].setScissor(0, scissor);
    }

    command_buffer_[image_index].draw(3, 1, 0, 0);

    command_buffer_[image_index].endRendering();

    transitionImageLayout(
        command_buffer_[image_index],
        swapchain_images_[image_index],
    vk::ImageLayout::eColorAttachmentOptimal,
        vk::ImageLayout::ePresentSrcKHR,
        vk::AccessFlagBits2::eColorAttachmentWrite,
        {},
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        vk::PipelineStageFlagBits2::eBottomOfPipe
    );

    command_buffer_[image_index].end();
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
    recordCommandBuffers();
}

void HelloTriangleApplicationCpp::drawFrame() {
    while (vk::Result::eTimeout == device_.waitForFences(*in_flight_fences_[current_frame_], vk::True, std::numeric_limits<uint64_t>::max()))
    {}
    auto [result, image_index] =
        swapchain_.acquireNextImage(std::numeric_limits<uint64_t>::max(), present_complete_semaphores_[semaphore_index_],nullptr);
    if (result == vk::Result::eErrorOutOfDateKHR) {
        recreateSwapChain();
        return;
    }
    if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
        throw std::runtime_error("Failed to acquire swap chain image !");

    device_.resetFences(*in_flight_fences_[current_frame_]);

    constexpr vk::PipelineStageFlags wait_destination_stage_mask{vk::PipelineStageFlagBits::eColorAttachmentOutput};
    const vk::SubmitInfo submit_info{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &*present_complete_semaphores_[semaphore_index_],
        .pWaitDstStageMask = &wait_destination_stage_mask,
        .commandBufferCount = 1,
        .pCommandBuffers = &*command_buffer_[image_index],
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &*render_finished_semaphores_[image_index],
    };

    graphic_queue.submit(submit_info, *in_flight_fences_[current_frame_]);

    const vk::PresentInfoKHR present_info{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &*render_finished_semaphores_[image_index],
        .swapchainCount = 1,
        .pSwapchains = &*swapchain_,
        .pImageIndices = &image_index,
    };

    result = present_queue.presentKHR(present_info);
    if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || framebufferResized) {
        framebufferResized = false;
        recreateSwapChain();
    }
    else if (result != vk::Result::eSuccess)
        throw std::runtime_error("Failed to present swapchain image !");

    semaphore_index_ = (semaphore_index_ + 1) % present_complete_semaphores_.size();
    current_frame_ = (current_frame_ + 1) % MAX_FRAME_IN_FLIGHT;
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
    std::vector buffer{readShader(shaderPath / "triangle_slang.spv")};

    const vk::ShaderModuleCreateInfo shader_module_create_info{
        .codeSize = buffer.size() * sizeof(char),
        .pCode = reinterpret_cast<uint32_t *>(buffer.data()),
    };

    vk::raii::ShaderModule shader_module{device_, shader_module_create_info};

    return shader_module;
}

void HelloTriangleApplicationCpp::framebufferResizeCallback(GLFWwindow *window, int width, int height) {
    auto app = reinterpret_cast<HelloTriangleApplicationCpp *>(glfwGetWindowUserPointer(window));
    app->framebufferResized = true;
}
