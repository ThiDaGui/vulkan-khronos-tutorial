#include "image.hh"

namespace vk_tutorial::vk_types
{
Image::Image(const vk::raii::Device& device, const VmaAllocator& vma_allocator,
             const vk::ImageCreateInfo& image_create_info, const VmaAllocationCreateInfo& alloc_create_info,
             vk::ImageViewCreateInfo& image_view_create_info)
    : image_extent_{image_create_info.extent}
    , image_format_{image_create_info.format}
{
    VkImage image;
    vmaCreateImage(vma_allocator, image_create_info, &alloc_create_info, &image, &image_memory_, nullptr);
    image_ = vk::raii::Image{device, image};
    image_view_create_info.image = image_;
    image_view_ = device.createImageView(image_view_create_info);
}

Image::Image(Image&& other) noexcept
{
    std::swap(image_, other.image_);
    std::swap(image_view_, other.image_view_);
    std::swap(image_memory_, other.image_memory_);
    image_extent_ = other.image_extent_;
    image_format_ = other.image_format_;
}

Image& Image::operator=(Image&& other) noexcept
{
    std::swap(image_, other.image_);
    std::swap(image_view_, other.image_view_);
    std::swap(image_memory_, other.image_memory_);
    image_extent_ = other.image_extent_;
    image_format_ = other.image_format_;
    return *this;
}
}
