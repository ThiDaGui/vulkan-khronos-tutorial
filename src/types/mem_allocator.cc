#include "mem_allocator.hh"

#include <vulkan/vulkan.hpp>

namespace vk_tutorial::vk_types
{
MemAllocator::MemAllocator(const VmaAllocatorCreateInfo* create_info)
{
    vmaCreateAllocator(create_info, &vma_allocator);
}

MemAllocator::MemAllocator(MemAllocator&& other) noexcept
{
    swap(other);
}

MemAllocator& MemAllocator::operator=(MemAllocator&& other) noexcept
{
    swap(other);
    return *this;
}

MemAllocator::~MemAllocator()
{
    vmaDestroyAllocator(vma_allocator);
}

void MemAllocator::swap(MemAllocator& other) noexcept
{
    std::swap(this->vma_allocator, other.vma_allocator);
}
}
