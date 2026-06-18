#include "buffer.hh"

namespace vk_tutorial::vk_types
{
Buffer::Buffer(VmaAllocator vma_allocator,
               const vk::DeviceSize buffer_size,
               const vk::BufferUsageFlags buffer_usage,
               const VmaMemoryUsage memory_usage,
               const VmaAllocationCreateFlags allocation_flags)
    : vma_allocator{vma_allocator}
    , buffer_size{buffer_size}
{
    vk::BufferCreateInfo buffer_create_info = {
        .size = buffer_size,
        .usage = buffer_usage,
        .sharingMode = vk::SharingMode::eExclusive,
    };

    const VmaAllocationCreateInfo allocation_create_info = {
        .flags = allocation_flags,
        .usage = memory_usage,
    };

    VmaAllocationInfo allocation_info{};
    VkBuffer vk_buffer;
    if (VK_SUCCESS != vmaCreateBuffer(vma_allocator, buffer_create_info, &allocation_create_info, &vk_buffer,
                                      &buffer_memory, &allocation_info))
        throw std::runtime_error("failed to create buffer!");

    buffer = vk_buffer;

    VkMemoryPropertyFlags mem_prop_flags;
    vmaGetAllocationMemoryProperties(vma_allocator, buffer_memory, &mem_prop_flags);

    if (mem_prop_flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
        buffer_mapped = allocation_info.pMappedData;
}

Buffer::~Buffer()
{
    if (!vma_allocator)
        return;

    vmaDestroyBuffer(vma_allocator, buffer, buffer_memory);
}

Buffer::Buffer(Buffer&& rhs) noexcept
{
    swap(rhs);
}

Buffer& Buffer::operator=(Buffer&& rhs) noexcept
{
    swap(rhs);
    return *this;
}

void Buffer::swap(Buffer& rhs) noexcept
{
    std::swap(buffer, rhs.buffer);
    std::swap(buffer_memory, rhs.buffer_memory);
    std::swap(buffer_mapped, rhs.buffer_mapped);
    std::swap(vma_allocator, rhs.vma_allocator);
    std::swap(buffer_size, rhs.buffer_size);
}

void Buffer::update(const void* data, const size_t size) const
{
    if (!buffer_mapped)
        throw std::runtime_error("Failed to update buffer : buffer is not host visible");
    if (size > buffer_size)
        throw std::runtime_error("Failed to update buffer : size > buffer_size");
    memcpy(buffer_mapped, data, size);
    vmaFlushAllocation(vma_allocator, buffer_memory, 0, size);
}
}
