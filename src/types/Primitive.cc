#include "Primitive.hh"

#include "mem_allocator.hh"
#include "engine/core.hh"


namespace vk_tutorial::vk_types
{
void Primitive::init(const Core& core, const std::span<const Vertex> vertices)
{
    vertex_buffer = vk_types::TypedBuffer<Vertex>{
        core.vma_allocator.vma_allocator,
        vertices.size(),
        vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eStorageBuffer |
        vk::BufferUsageFlagBits::eTransferDst,
        VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
        0
    };
    vertex_buffer_address = core.device.getBufferAddress({.buffer = vertex_buffer.buffer});

    const TypedBuffer<Vertex> staging{
        core.vma_allocator.vma_allocator,
        vertices.size(),
        vk::BufferUsageFlagBits::eTransferSrc,
        VMA_MEMORY_USAGE_AUTO,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT
    };


    staging.update(vertices);

    core.immediate_command_buffer.begin({});
    core.immediate_command_buffer.copyBuffer(
        staging.buffer, vertex_buffer.buffer,
        vk::BufferCopy{
            .srcOffset = 0, .dstOffset = 0, .size = vertices.size_bytes()
        });
    core.immediate_command_buffer.end();
    const vk::SubmitInfo submit_info = {
        .commandBufferCount = 1,
        .pCommandBuffers = &*core.immediate_command_buffer,
    };

    core.graphics_queue.submit(submit_info);

    core.device.waitIdle();
}
} // vk_tutorial::vk_types
