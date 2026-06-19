#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>

#include "utils/types.hh"

namespace vk_tutorial::vk_types
{
class Buffer : NonCopyable
{
public:
    vk::Buffer buffer{};
    VmaAllocation buffer_memory{};
    void* buffer_mapped{nullptr};
    VmaAllocator vma_allocator{};
    size_t buffer_size{};

    Buffer() = default;

    explicit Buffer(VmaAllocator vma_allocator,
                    vk::DeviceSize buffer_size,
                    vk::BufferUsageFlags buffer_usage,
                    VmaMemoryUsage memory_usage,
                    VmaAllocationCreateFlags allocation_flags);

    virtual ~Buffer();

    Buffer(Buffer&& rhs) noexcept;

    Buffer& operator=(Buffer&& rhs) noexcept;

    void swap(Buffer& rhs) noexcept;

    void update(const void* data, size_t size) const;
};
}
