#pragma once

#include <vulkan/vulkan.hpp>

#include "utils/types.hh"
#include "vk_mem_alloc/vk_mem_alloc.h"

namespace vk_tutorial::vk_types
{
class Buffer : NonCopyable
{
public:
    vk::Buffer buffer_{};
    VmaAllocation buffer_memory_{};
    void* buffer_mapped_{nullptr};
    VmaAllocator vma_allocator_{};

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
