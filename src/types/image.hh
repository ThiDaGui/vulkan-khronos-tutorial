#pragma once

#include <vk_mem_alloc/vma_usage.hh>
#include <vulkan/vulkan_raii.hpp>

namespace vk_tutorial::vk_types
{
class Image
{
    vk::Extent3D image_extent_{};
    vk::Format image_format_{};
    vk::raii::Image image_{nullptr};
    vk::raii::ImageView image_view_{nullptr};
    VmaAllocation image_memory_{nullptr};

public:
    Image() = default;

    explicit Image(const vk::raii::Device& device,
                   const VmaAllocator& vma_allocator,
                   const vk::ImageCreateInfo& image_create_info,
                   const VmaAllocationCreateInfo& alloc_create_info,
                   vk::ImageViewCreateInfo& image_view_create_info);

    ~Image() = default;

    Image(const Image& other) = delete;
    Image operator=(const Image& other) = delete;

    Image(Image&& other) noexcept;
    Image& operator=(Image&& other) noexcept;

};
}
