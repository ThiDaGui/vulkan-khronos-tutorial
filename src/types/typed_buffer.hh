#pragma once
#include "buffer.hh"

namespace vk_tutorial::vk_types
{
template <typename T>
class TypedBuffer : public Buffer
{
public:
    explicit TypedBuffer(VmaAllocator vma_allocator,
                         std::size_t nb_elements,
                         vk::BufferUsageFlags buffer_usage,
                         VmaMemoryUsage memory_usage,
                         VmaAllocationCreateFlags allocation_flags);

    TypedBuffer() = default;

    TypedBuffer(TypedBuffer&& rhs) noexcept;

    TypedBuffer& operator=(TypedBuffer&& rhs) noexcept;

    // ReSharper disable once CppHidingFunction
    void update(std::span<const T> data) const;
};

template <typename T>
TypedBuffer<T>::TypedBuffer(const VmaAllocator vma_allocator, // NOLINT(*-misplaced-const)
                            const std::size_t nb_elements,
                            const vk::BufferUsageFlags buffer_usage,
                            const VmaMemoryUsage memory_usage,
                            const VmaAllocationCreateFlags allocation_flags)
    : Buffer{vma_allocator, sizeof(T) * nb_elements, buffer_usage, memory_usage, allocation_flags}
{}

template <typename T>
TypedBuffer<T>::TypedBuffer(TypedBuffer&& rhs) noexcept
{
    Buffer::swap(rhs);
}

template <typename T>
TypedBuffer<T>& TypedBuffer<T>::operator=(TypedBuffer&& rhs) noexcept
{
    Buffer::swap(rhs);
    return *this;
}

template <typename T>
void TypedBuffer<T>::update(const std::span<const T> data) const
{
    Buffer::update(data.data(), data.size() * sizeof(T));
}
}
