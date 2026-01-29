#include "vk_engine.hh"

#include "config.hh"
#include "required_queue_family_indices.hh"

namespace vk_tutorial
{
VkEngine::VkEngine()
{
    window_system_.init(window_extent.width, window_extent.height, "Vulkan Tutorial");

    std::vector<const char*> instance_extensions = WindowSystem::getRequiredExtensions();
    std::vector<const char*> instance_layers{};

#ifndef NDEBUG
    instance_extensions.push_back(vk::EXTDebugUtilsExtensionName);
    instance_layers.push_back("VK_LAYER_KHRONOS_validation");
#endif

    core_.init(instance_extensions, instance_layers, REQUIRED_DEVICE_EXTENSIONS, window_system_);
    swapchain_.init(core_, window_system_);

    vk::FenceCreateInfo fence_create_info = {
        .flags = vk::FenceCreateFlagBits::eSignaled,
    };
    in_flight_fences_.reserve(FRAME_OVERLAP);
    for (auto i = 0; i < FRAME_OVERLAP; i++)
    {
        in_flight_fences_.emplace_back(core_.device_, fence_create_info);
    }

    std::array color_attachments = {swapchain_.image_format};
    pipeline = vk_types::Pipeline::CreateGraphicPipeline(core_.device_, shaderPath/"triangle_slang.spv", color_attachments);

    is_initialized = true;
}

VkEngine::~VkEngine() = default;

void VkEngine::run()
{
    while (!window_system_.shouldClose())
    {
        draw();
    }
    core_.device_.waitIdle();
}

void VkEngine::draw()
{
    if (vk::Result::eSuccess != core_.device_.waitForFences(*in_flight_fences_[fence_index_], vk::True,
                                                            std::numeric_limits<uint64_t>::max()))
        throw std::runtime_error("Error while waiting for fence !");

    core_.device_.resetFences(*in_flight_fences_[fence_index_]);

    const uint32_t image_index = swapchain_.Acquire(semaphore_index_);

    auto & [image_view, command_buffer, is_presentable_semaphore] = swapchain_.frames_data[image_index];

    {
        command_buffer.begin({});

        vk::ImageMemoryBarrier2 memory_barrier2 = {
            .srcStageMask = vk::PipelineStageFlagBits2::eNone,
            .srcAccessMask = vk::AccessFlagBits2::eNone,
            .dstStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
            .dstAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite,
            .oldLayout = vk::ImageLayout::eUndefined,
            .newLayout = vk::ImageLayout::eColorAttachmentOptimal,
            .image = swapchain_.images[image_index],
            .subresourceRange = {
                .aspectMask = vk::ImageAspectFlagBits::eColor,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
        };

        command_buffer.pipelineBarrier2(
            {
                .imageMemoryBarrierCount = 1,
                .pImageMemoryBarriers = &memory_barrier2
            });

        constexpr vk::ClearValue clear_color = vk::ClearColorValue{0.5f, 0.5f, 0.5f, 1.0f};
        vk::RenderingAttachmentInfo attachment_info = {
            .imageView = image_view,
            .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
            .loadOp = vk::AttachmentLoadOp::eClear,
            .storeOp = vk::AttachmentStoreOp::eStore,
            .clearValue = clear_color,
        };
        const vk::RenderingInfo rendering_info = {
            .renderArea = {.offset = {0, 0}, .extent = swapchain_.extent},
            .layerCount = 1,
            .colorAttachmentCount = 1,
            .pColorAttachments = &attachment_info
        };

        command_buffer.beginRendering(rendering_info);
        command_buffer.endRendering();

        memory_barrier2 = {
            .srcStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
            .srcAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite,
            .dstStageMask = vk::PipelineStageFlagBits2::eNone,
            .dstAccessMask = vk::AccessFlagBits2::eNone,
            .oldLayout = vk::ImageLayout::eColorAttachmentOptimal,
            .newLayout = vk::ImageLayout::ePresentSrcKHR,
            .image = swapchain_.images[image_index],
            .subresourceRange = {
                .aspectMask = vk::ImageAspectFlagBits::eColor,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
        };

        command_buffer.pipelineBarrier2(
            {
                .imageMemoryBarrierCount = 1,
                .pImageMemoryBarriers = &memory_barrier2
            });

        command_buffer.end();
    }

    constexpr vk::PipelineStageFlags wait_destination_stage_mask{vk::PipelineStageFlagBits::eColorAttachmentOutput};

    const vk::SubmitInfo submit_info = {
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &*swapchain_.frame_acquired_semaphores[semaphore_index_],
        .pWaitDstStageMask = &wait_destination_stage_mask,
        .commandBufferCount = 1,
        .pCommandBuffers = &*command_buffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &*is_presentable_semaphore,
    };

    core_.graphics_queue.submit(submit_info, in_flight_fences_[fence_index_]);

    swapchain_.Present(core_.present_queue, image_index);

    semaphore_index_ = (semaphore_index_ + 1) % swapchain_.image_count;
    fence_index_ = (fence_index_ + 1) % FRAME_OVERLAP;
}
}
