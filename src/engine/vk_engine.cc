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
    pipeline = vk_types::Pipeline::CreateGraphicPipeline(core_.device_, shaderPath / "triangle_slang.spv",
                                                         color_attachments);

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
                                                            -1))
        throw std::runtime_error("Error while waiting for fence !");

    core_.device_.resetFences(*in_flight_fences_[fence_index_]);

    auto [result, image_index] = swapchain_.Acquire(semaphore_index_);
    if (vk::Result::eErrorOutOfDateKHR == result)
    {
        swapchain_.recreate(core_, window_system_);
        return;
    }
    if (vk::Result::eSuccess != result && vk::Result::eSuboptimalKHR != result)
        throw std::runtime_error("Failed to acquire swapchain image !");

    auto& [image_view, command_buffer, is_presentable_semaphore] = swapchain_.frames_data[image_index];

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
        {
            const vk::Viewport viewport = {
                0.0f,
                0.0f,
                static_cast<float>(swapchain_.extent.width),
                static_cast<float>(swapchain_.extent.height),
                0.0f,
                1.0f
            };

            const vk::Rect2D scissor{{0, 0}, swapchain_.extent};

            command_buffer.setViewport(0, viewport);
            command_buffer.setScissor(0, scissor);
        }
        command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.getPipeline());
        command_buffer.draw(3, 1, 0, 0);
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

    result = swapchain_.Present(core_.present_queue, image_index);
    if (vk::Result::eErrorOutOfDateKHR == result || vk::Result::eSuboptimalKHR == result || window_system_.resized)
    {
        window_system_.resized = false;
        swapchain_.recreate(core_, window_system_);
    }

    semaphore_index_ = (semaphore_index_ + 1) % swapchain_.image_count;
    fence_index_ = (fence_index_ + 1) % FRAME_OVERLAP;
}
}
