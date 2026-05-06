#include "vk_engine.hh"

#include <glm/gtc/matrix_transform.hpp>

#include "config.hh"
#include "types/buffer.hh"
#include "utils/descriptor_set_layout_builder.hh"

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

    color_render_target_.reserve(FRAME_OVERLAP);
    for (size_t i = 0; i < FRAME_OVERLAP; i++)
    {
        color_render_target_.emplace_back(
            core_.device_,
            core_.vma_allocator.vma_allocator,
            vk::Format::eR16G16B16A16Sfloat,
            swapchain_.extent,
            vk::SampleCountFlagBits::e1,
            vk_types::Image::Usage::eColorAttachment,
            vk::ImageAspectFlagBits::eColor
        );
    }

    std::array descriptor_entries{
        vk_types::DescriptorAllocator::DescriptorEntry{vk::DescriptorType::eUniformBuffer, 1}
    };
    descriptor_set_layout_ = DescriptorSetLayoutBuilder()
                             .AddBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex)
                             .Build(core_.device_);

    descriptor_allocator_ = vk_types::DescriptorAllocator{core_.device_, FRAME_OVERLAP, descriptor_entries};

    descriptor_set_ = {
        descriptor_allocator_.allocate(core_.device_, descriptor_set_layout_),
        descriptor_allocator_.allocate(core_.device_, descriptor_set_layout_),
    };


    view_proj_uniform_ = {
        vk_types::Buffer{
            core_.vma_allocator.vma_allocator, sizeof(CameraData),
            vk::BufferUsageFlagBits::eUniformBuffer,
            VMA_MEMORY_USAGE_AUTO,
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
        },
        vk_types::Buffer{
            core_.vma_allocator.vma_allocator, sizeof(CameraData),
            vk::BufferUsageFlagBits::eUniformBuffer,
            VMA_MEMORY_USAGE_AUTO,
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
        },
    };
    for (size_t i = 0; i < FRAME_OVERLAP; i++)
    {
        vk::DescriptorBufferInfo info = {
            view_proj_uniform_[i].buffer_, 0, sizeof(CameraData)
        };
        vk::WriteDescriptorSet write_descriptor_set = {
            .dstSet = descriptor_set_[i],
            .dstBinding = 0,
            .descriptorCount = 1,
            .descriptorType = vk::DescriptorType::eUniformBuffer,
            .pBufferInfo = &info,
        };
        core_.device_.updateDescriptorSets(write_descriptor_set, {});
    }

    std::array color_attachments = {color_render_target_[0].image_format_};

    vk::PushConstantRange range{vk::ShaderStageFlagBits::eVertex, 0, sizeof(vk::DeviceAddress)};
    const vk::PipelineLayoutCreateInfo pipeline_layout_create_info = {
        .setLayoutCount = 1,
        .pSetLayouts = &*descriptor_set_layout_,
        .pushConstantRangeCount = 1,
        .pPushConstantRanges = &range,
    };

    const auto pipeline_layout = (*core_.device_).createPipelineLayout(pipeline_layout_create_info);
    pipeline_ = vk_types::Pipeline::CreateGraphicPipeline(core_.device_, shaderPath / "triangle_slang.spv",
                                                          color_attachments, pipeline_layout);

    constexpr std::array mesh_array{
        Vertex{.position = { 0.5f, 0.0f, -0.5f}, .color = {1.0f, 0.0f, 0.0f}},
        Vertex{.position = { 0.5f, 0.0f,  0.5f}, .color = {1.0f, 1.0f, 0.0f}},
        Vertex{.position = {-0.5f, 0.0f,  0.5f}, .color = {0.0f, 1.0f, 0.0f}},
        Vertex{.position = { 0.5f, 0.0f, -0.5f}, .color = {1.0f, 0.0f, 0.0f}},
        Vertex{.position = {-0.5f, 0.0f,  0.5f}, .color = {0.0f, 1.0f, 0.0f}},
        Vertex{.position = {-0.5f, 0.0f, -0.5f}, .color = {0.0f, 0.0f, 0.0f}},
    };

    mesh.vertex_buffer = vk_types::TypedBuffer<Vertex>{
        core_.vma_allocator.vma_allocator,
        mesh_array.size(),
        vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eStorageBuffer |
        vk::BufferUsageFlagBits::eTransferDst,
        VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
        VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT
    };
    mesh.vertex_buffer_address = core_.device_.getBufferAddress({.buffer = mesh.vertex_buffer.buffer_});

    const vk_types::TypedBuffer<Vertex> staging{
        core_.vma_allocator.vma_allocator,
        mesh_array.size(),
        vk::BufferUsageFlagBits::eTransferSrc,
        VMA_MEMORY_USAGE_AUTO,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT
    };


    staging.update(mesh_array);

    vk::CommandBufferAllocateInfo cmAI{
        .commandPool = core_.graphics_command_pool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1
    };
    vk::raii::CommandBuffer command_buffer = std::move(core_.device_.allocateCommandBuffers(cmAI).front());
    command_buffer.begin({});
    command_buffer.copyBuffer(staging.buffer_, mesh.vertex_buffer.buffer_, vk::BufferCopy{.srcOffset = 0, .dstOffset = 0, .size = mesh_array.size() * sizeof(Vertex)});
    command_buffer.end();
    const vk::SubmitInfo submit_info = {
        .commandBufferCount = 1,
        .pCommandBuffers = &*command_buffer,
    };

    core_.graphics_queue.submit(submit_info);
    core_.device_.waitIdle();

    is_initialized = true;
}

VkEngine::~VkEngine() = default;

void VkEngine::run()
{
    while (!window_system_.shouldClose())
    {
        update();
        draw();
    }
    core_.device_.waitIdle();
}

void VkEngine::update() const
{
    static auto start_time = std::chrono::high_resolution_clock::now();

    const auto current_time = std::chrono::high_resolution_clock::now();
    const float time = std::chrono::duration<float>(current_time - start_time).count();
    CameraData camera_data{};
    camera_data.view_matrix = glm::lookAt(glm::vec3{2.0f * glm::sin(time), 2.0f * glm::cos(time), 0.0f},
                                 glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{0.0f, 0.0f, 1.0f});
    camera_data.projection_matrix = glm::perspective(glm::radians(30.0f),
                                            static_cast<float>(swapchain_.extent.width) / static_cast<float>(swapchain_.
                                                extent.height), 0.1f, 10.0f);
    camera_data.projection_matrix[1][1] *= -1;

    view_proj_uniform_[in_flight_index_].update(&camera_data, sizeof(camera_data));
}

void VkEngine::draw()
{
    if (vk::Result::eSuccess != core_.device_.waitForFences(*in_flight_fences_[in_flight_index_], vk::True,
                                                            -1))
        throw std::runtime_error("Error while waiting for fence !");

    core_.device_.resetFences(*in_flight_fences_[in_flight_index_]);

    auto [result, image_index] = swapchain_.Acquire();
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

        color_render_target_[in_flight_index_].transition(command_buffer, vk::PipelineStageFlagBits2::eNone,
                                                          vk::AccessFlagBits2::eNone,
                                                          vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                                                          vk::AccessFlagBits2::eColorAttachmentWrite,
                                                          vk::ImageLayout::eUndefined,
                                                          vk::ImageLayout::eColorAttachmentOptimal,
                                                          vk::ImageAspectFlagBits::eColor);

        constexpr vk::ClearValue clear_color = vk::ClearColorValue{0.5f, 0.5f, 0.5f, 1.0f};
        vk::RenderingAttachmentInfo attachment_info = {
            .imageView = color_render_target_[in_flight_index_].image_view_,
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
        command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline_.getPipeline());
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
        command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline_.getLayout(), 0,
                                          descriptor_set_[in_flight_index_], nullptr);
        command_buffer.pushConstants<vk::DeviceAddress>(pipeline_.getLayout(), vk::ShaderStageFlagBits::eVertex, 0, mesh.vertex_buffer_address );
        command_buffer.draw(6, 1, 0, 0);
        command_buffer.endRendering();

        color_render_target_[in_flight_index_].transition(
            command_buffer,
            vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::AccessFlagBits2::eColorAttachmentWrite,
            vk::PipelineStageFlagBits2::eTransfer, vk::AccessFlagBits2::eTransferRead,
            vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eTransferSrcOptimal,
            vk::ImageAspectFlagBits::eColor);


        vk_types::Image::transition(command_buffer, swapchain_.images[image_index],
                                    vk::PipelineStageFlagBits2::eNone, vk::AccessFlagBits2::eNone,
                                    vk::PipelineStageFlagBits2::eTransfer, vk::AccessFlagBits2::eTransferWrite,
                                    vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal,
                                    vk::ImageAspectFlagBits::eColor);

        color_render_target_[in_flight_index_].copy(command_buffer, swapchain_.images[image_index], swapchain_.extent);

        vk_types::Image::transition(command_buffer, swapchain_.images[image_index],
                                    vk::PipelineStageFlagBits2::eTransfer, vk::AccessFlagBits2::eTransferWrite,
                                    vk::PipelineStageFlagBits2::eNone, vk::AccessFlagBits2::eNone,
                                    vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::ePresentSrcKHR,
                                    vk::ImageAspectFlagBits::eColor);

        command_buffer.end();
    }

    constexpr vk::PipelineStageFlags wait_destination_stage_mask{vk::PipelineStageFlagBits::eColorAttachmentOutput};

    std::array wait_semaphore = {swapchain_.GetCurrentSemaphore()};
    const vk::SubmitInfo submit_info = {
        .waitSemaphoreCount = wait_semaphore.size(),
        .pWaitSemaphores = wait_semaphore.data(),
        .pWaitDstStageMask = &wait_destination_stage_mask,
        .commandBufferCount = 1,
        .pCommandBuffers = &*command_buffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &*is_presentable_semaphore,
    };

    core_.graphics_queue.submit(submit_info, in_flight_fences_[in_flight_index_]);

    result = swapchain_.Present(core_.present_queue, image_index);
    if (vk::Result::eErrorOutOfDateKHR == result || vk::Result::eSuboptimalKHR == result || window_system_.resized)
    {
        window_system_.resized = false;
        swapchain_.recreate(core_, window_system_);
        color_render_target_.clear();
        color_render_target_.reserve(FRAME_OVERLAP);
        for (size_t i = 0; i < FRAME_OVERLAP; i++)
        {
            color_render_target_.emplace_back(
                core_.device_,
                core_.vma_allocator.vma_allocator,
                vk::Format::eR16G16B16A16Sfloat,
                swapchain_.extent,
                vk::SampleCountFlagBits::e1,
                vk_types::Image::Usage::eColorAttachment,
                vk::ImageAspectFlagBits::eColor
            );
        }
    }
    if (vk::Result::eSuccess != result)
        throw std::runtime_error("Failed to present swapchain image !");


    in_flight_index_ = (in_flight_index_ + 1) % FRAME_OVERLAP;
}
}
