#include "swapchain.hh"

#include "core.hh"
#include "required_queue_family_indices.hh"
#include "window_system.hh"

namespace vk_tutorial
{
void Swapchain::init(const Core& core,
                     const WindowSystem& window_system)
{
    const vk::SurfaceCapabilitiesKHR surface_capabilities = core.physical_device_.getSurfaceCapabilitiesKHR(
        core.surface_);
    const std::vector<vk::SurfaceFormatKHR> surface_formats = core.physical_device_.getSurfaceFormatsKHR(
        core.surface_);
    const std::vector<vk::PresentModeKHR> present_modes = core.physical_device_.getSurfacePresentModesKHR(
        core.surface_);

    extent = chooseExtent2D(window_system, surface_capabilities);
    min_image_count = chooseMinImageCount(surface_capabilities);
    const vk::SurfaceFormatKHR surface_format = chooseSurfaceFormat(surface_formats);
    const vk::PresentModeKHR present_mode = choosePresentMode(present_modes);

    vk::SwapchainCreateInfoKHR swapchain_create_info = {
        .surface = core.surface_,
        .minImageCount = min_image_count,
        .imageFormat = surface_format.format,
        .imageColorSpace = surface_format.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
        .preTransform = surface_capabilities.currentTransform,
        .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
        .presentMode = present_mode,
        .clipped = vk::True,
        .oldSwapchain = nullptr,
    };

    const std::array queue_family_indices = {
        core.queue_family_indices.graphics_queue_family.value(),
        core.queue_family_indices.present_queue_family.value(),
    };

    if (core.queue_family_indices.graphics_queue_family.value() ==
        core.queue_family_indices.present_queue_family.value())
    {
        swapchain_create_info.imageSharingMode = vk::SharingMode::eExclusive;
    }
    else
    {
        swapchain_create_info.imageSharingMode = vk::SharingMode::eConcurrent;
        swapchain_create_info.queueFamilyIndexCount = static_cast<uint32_t>(queue_family_indices.size());
        swapchain_create_info.pQueueFamilyIndices = queue_family_indices.data();
    }

    vk_swapchain = vk::raii::SwapchainKHR{core.device_, swapchain_create_info};
    image_format = surface_format.format;
    images = vk_swapchain.getImages();

    image_count = images.size();

    frames_data.resize(image_count);
    frame_acquired_semaphores.reserve(image_count);

    static constexpr vk::ImageSubresourceRange subresource_range{
        .aspectMask = vk::ImageAspectFlagBits::eColor,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1,
    };

    vk::ImageViewCreateInfo image_view_create_info{
        .viewType = vk::ImageViewType::e2D,
        .format = image_format,
        .subresourceRange = subresource_range
    };
    const vk::CommandBufferAllocateInfo graphics_command_buffer_allocate_info = {
        .commandPool = core.graphics_command_pool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1,
    };

    for (uint32_t i = 0; i < image_count; i++)
    {
        image_view_create_info.image = images[i];
        frames_data[i].image_view = core.device_.createImageView(image_view_create_info);
        frames_data[i].command_buffer = std::move(vk::raii::CommandBuffers{
            core.device_, graphics_command_buffer_allocate_info
        }.front());
        frames_data[i].is_presentable_semaphore = core.device_.createSemaphore({});

        frame_acquired_semaphores.emplace_back(core.device_, vk::SemaphoreCreateInfo{});
    }
}

uint32_t Swapchain::Acquire(const uint32_t semaphore_index) const
{
    auto [result, image_index] =
        vk_swapchain.acquireNextImage(-1, *frame_acquired_semaphores[semaphore_index], nullptr);

    if (vk::Result::eSuccess != result)
        throw std::runtime_error("Failed to acquire swapchain image!");

    return image_index;
}

void Swapchain::Present(const vk::raii::Queue& present_queue, uint32_t image_index) const
{
    const vk::PresentInfoKHR present_info = {
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &*frames_data[image_index].is_presentable_semaphore,
        .swapchainCount = 1,
        .pSwapchains = &*vk_swapchain,
        .pImageIndices = &image_index,
    };

    vk::Result result = present_queue.presentKHR(present_info);

    if (vk::Result::eSuccess != result)
        throw std::runtime_error("Failed to present swapchain image!");
}


vk::Extent2D Swapchain::chooseExtent2D(const WindowSystem& window_system,
                                       const vk::SurfaceCapabilitiesKHR& surface_capabilities)
{
    if (surface_capabilities.currentExtent.width != std::numeric_limits<std::uint32_t>::max())
        return surface_capabilities.currentExtent;

    const auto [width, height] = window_system.getExtent();

    return {
        std::clamp(width, surface_capabilities.minImageExtent.width,
                   surface_capabilities.maxImageExtent.width),
        std::clamp(height, surface_capabilities.minImageExtent.height,
                   surface_capabilities.maxImageExtent.height)
    };
}

uint32_t Swapchain::chooseMinImageCount(const vk::SurfaceCapabilitiesKHR& surface_capabilities)
{
    const uint32_t image_count = surface_capabilities.minImageCount + 1;
    if (surface_capabilities.maxImageCount > 0 && image_count > surface_capabilities.maxImageCount)
        return surface_capabilities.maxImageCount;
    return image_count;
}

vk::SurfaceFormatKHR Swapchain::chooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& surface_formats)
{
    for (const auto& available_surface_format : surface_formats)
    {
        if (available_surface_format.format == vk::Format::eB8G8R8A8Srgb && available_surface_format.colorSpace ==
            vk::ColorSpaceKHR::eSrgbNonlinear)
            return available_surface_format;
    }
    return surface_formats[0];
}

vk::PresentModeKHR Swapchain::choosePresentMode(const std::vector<vk::PresentModeKHR>& present_modes)
{
    for (const auto& present_mode : present_modes)
    {
        if (present_mode == vk::PresentModeKHR::eMailbox)
            return present_mode;
    }
    return vk::PresentModeKHR::eFifo;
}
}
