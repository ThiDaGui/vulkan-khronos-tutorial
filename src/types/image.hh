#pragma once

#include <vk_mem_alloc/vma_usage.hh>
#include <vulkan/vulkan_raii.hpp>

#include "utils/types.hh"

namespace vk_tutorial::vk_types
{
class Image final : NonCopyable
{
    vk::Extent3D image_extent_{};
    vk::Format image_format_{};
    vk::raii::Image image_{nullptr};
    vk::raii::ImageView image_view_{nullptr};
    VmaAllocation image_memory_{nullptr};

public:
    Image() = default;

    explicit Image(const vk::raii::Device& device,
                   VmaAllocator vma_allocator,
                   const vk::ImageCreateInfo& image_create_info,
                   const VmaAllocationCreateInfo& alloc_create_info,
                   vk::ImageViewCreateInfo& image_view_create_info);

    ~Image() = default;

    Image(Image&& other) noexcept;

    Image& operator=(Image&& other) noexcept;

    void swap(Image& other) noexcept;
};
}
