#pragma once

#include <vk_mem_alloc.h>

#include "utils/types.hh"

namespace vk_tutorial::vk_types
{
struct MemAllocator final : NonCopyable
{
    VmaAllocator vma_allocator{};

    constexpr MemAllocator() = default;

    explicit MemAllocator(const VmaAllocatorCreateInfo* create_info);

    MemAllocator(MemAllocator&& other) noexcept;

    MemAllocator& operator=(MemAllocator&& other) noexcept;

    ~MemAllocator();

    void swap(MemAllocator& other) noexcept;
};
}
