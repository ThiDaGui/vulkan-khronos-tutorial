#include "buffer.hh"

namespace vk_tutorial::vk_types
{
Buffer::Buffer(const VmaAllocator vma_allocator,
               const vk::DeviceSize buffer_size,
               const vk::BufferUsageFlags buffer_usage,
               const VmaMemoryUsage memory_usage,
               const VmaAllocationCreateFlags allocation_flags)
    : vma_allocator_{vma_allocator}
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

    VkBuffer buffer;
    if (VK_SUCCESS != vmaCreateBuffer(vma_allocator_, buffer_create_info, &allocation_create_info, &buffer,
                                      &buffer_memory_, nullptr))
        throw std::runtime_error("failed to create buffer!");

    buffer_ = buffer;

    VkMemoryPropertyFlags memPropFlags;
    vmaGetAllocationMemoryProperties(vma_allocator_, buffer_memory_, &memPropFlags);

    if (memPropFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
        vmaMapMemory(vma_allocator_, buffer_memory_, &buffer_mapped_);
}

Buffer::~Buffer()
{
    if (!vma_allocator_)
        return;
    vmaUnmapMemory(vma_allocator_, buffer_memory_);
    vmaDestroyBuffer(vma_allocator_, buffer_, buffer_memory_);
}

Buffer::Buffer(Buffer&& other) noexcept
{
    swap(other);
}

Buffer& Buffer::operator=(Buffer&& other) noexcept
{
    swap(other);
    return *this;
}

void Buffer::swap(Buffer& rhs) noexcept
{
    std::swap(buffer_, rhs.buffer_);
    std::swap(buffer_memory_, rhs.buffer_memory_);
    std::swap(buffer_mapped_, rhs.buffer_mapped_);
    std::swap(vma_allocator_, rhs.vma_allocator_);
}

void Buffer::update(const void* data, const size_t size) const
{
    if (!buffer_mapped_)
        return;
    vmaCopyMemoryToAllocation(vma_allocator_, data, buffer_memory_, 0, size);
}
}
