#include "image.hh"

namespace vk_tutorial::vk_types
{
Image::Image(const vk::raii::Device& device,
             VmaAllocator vma_allocator,
             const vk::Format format,
             const vk::Extent2D extent_2d,
             const vk::ImageUsageFlags image_usage,
             const vk::SampleCountFlagBits sample_count,
             const VmaMemoryUsage memory_usage,
             const vk::ImageAspectFlags image_aspect)
    : image_extent_{extent_2d.width, extent_2d.height, 1}
    , image_format_{format}
    , vma_allocator_(vma_allocator)
{
    VkImage image;
    const vk::ImageCreateInfo image_create_info = {
        .imageType = vk::ImageType::e2D,
        .format = format,
        .extent = image_extent_,
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = sample_count,
        .tiling = vk::ImageTiling::eOptimal,
        .usage = image_usage,
    };

    const VmaAllocationCreateInfo alloc_create_info = {
        .usage = memory_usage,
        .requiredFlags = static_cast<VkMemoryPropertyFlags>(vk::MemoryPropertyFlagBits::eDeviceLocal),
    };

    vmaCreateImage(vma_allocator_, image_create_info, &alloc_create_info, &image, &image_memory_, nullptr);
    image_ = vk::raii::Image{device, image};
    vk::ImageViewCreateInfo image_view_create_info = {
        .image = image_,
        .viewType = vk::ImageViewType::e2D,
        .format = format,
        .components = {},
        .subresourceRange = vk::ImageSubresourceRange{image_aspect, 0, 1, 0, 1}
    };
    image_view_ = device.createImageView(image_view_create_info);
}

Image::~Image()
{
    vmaFreeMemory(vma_allocator_, image_memory_);
}

Image::Image(Image&& other) noexcept
{
    swap(other);
}

Image& Image::operator=(Image&& other) noexcept
{
    swap(other);
    return *this;
}

void Image::swap(Image& other) noexcept
{
    if (this == &other)
        return;
    std::swap(image_, other.image_);
    std::swap(image_view_, other.image_view_);
    std::swap(image_memory_, other.image_memory_);
    std::swap(image_extent_, other.image_extent_);
    std::swap(image_format_, other.image_format_);
}

void Image::transition(const vk::raii::CommandBuffer& command_buffer,
                       const vk::PipelineStageFlagBits2 src_stage_mask, const vk::AccessFlagBits2 src_access_mask,
                       const vk::PipelineStageFlagBits2 dst_stage_mask, const vk::AccessFlagBits2 dst_access_mask,
                       const vk::ImageLayout old_layout, const vk::ImageLayout new_layout,
                       const vk::ImageAspectFlags aspect_mask) const
{
    vk::ImageMemoryBarrier2 barrier = {
        .srcStageMask = src_stage_mask,
        .srcAccessMask = src_access_mask,
        .dstStageMask = dst_stage_mask,
        .dstAccessMask = dst_access_mask,
        .oldLayout = old_layout,
        .newLayout = new_layout,
        .image = image_,
        .subresourceRange = {
            .aspectMask = aspect_mask,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        }
    };
    command_buffer.pipelineBarrier2({.imageMemoryBarrierCount = 1, .pImageMemoryBarriers = &barrier});
}

void Image::transition(const vk::raii::CommandBuffer& command_buffer, vk::Image image,
                       vk::PipelineStageFlagBits2 src_stage_mask, vk::AccessFlagBits2 src_access_mask,
                       vk::PipelineStageFlagBits2 dst_stage_mask, vk::AccessFlagBits2 dst_access_mask,
                       vk::ImageLayout old_layout, vk::ImageLayout new_layout,
                       vk::ImageAspectFlags aspect_mask)
{
    vk::ImageMemoryBarrier2 barrier = {
        .srcStageMask = src_stage_mask,
        .srcAccessMask = src_access_mask,
        .dstStageMask = dst_stage_mask,
        .dstAccessMask = dst_access_mask,
        .oldLayout = old_layout,
        .newLayout = new_layout,
        .image = image,
        .subresourceRange = {
            .aspectMask = aspect_mask,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        }
    };
    command_buffer.pipelineBarrier2({.imageMemoryBarrierCount = 1, .pImageMemoryBarriers = &barrier});
}

void Image::copy(const vk::raii::CommandBuffer& command_buffer, const Image& dst_image) const
{
    const vk::ImageBlit2 blit_region = {
        .srcSubresource = {vk::ImageAspectFlagBits::eColor, 0, 0, 1},
        .srcOffsets = std::array{
            vk::Offset3D{0, 0, 0},
            vk::Offset3D{static_cast<int32_t>(image_extent_.width), static_cast<int32_t>(image_extent_.height), 1}
        },
        .dstSubresource = vk::ImageSubresourceLayers{vk::ImageAspectFlagBits::eColor, 0, 0, 1},
        .dstOffsets = std::array{
            vk::Offset3D{0, 0, 0},
            vk::Offset3D{
                static_cast<int32_t>(dst_image.image_extent_.width),
                static_cast<int32_t>(dst_image.image_extent_.height),
                1
            }
        }
    };

    const vk::BlitImageInfo2 blit_image_info2 = {
        .srcImage = image_,
        .srcImageLayout = vk::ImageLayout::eTransferSrcOptimal,
        .dstImage = dst_image.image_,
        .dstImageLayout = vk::ImageLayout::eTransferDstOptimal,
        .regionCount = 1,
        .pRegions = &blit_region,
        .filter = vk::Filter::eLinear
    };

    command_buffer.blitImage2(blit_image_info2);
}

void Image::copy(const vk::raii::CommandBuffer& command_buffer,
                 const vk::Image dst_image,
                 const vk::Extent2D extent) const
{
    const vk::ImageBlit2 blit_region = {
        .srcSubresource = {vk::ImageAspectFlagBits::eColor, 0, 0, 1},
        .srcOffsets = std::array{
            vk::Offset3D{0, 0, 0},
            vk::Offset3D{static_cast<int32_t>(image_extent_.width), static_cast<int32_t>(image_extent_.height), 1}
        },
        .dstSubresource = vk::ImageSubresourceLayers{vk::ImageAspectFlagBits::eColor, 0, 0, 1},
        .dstOffsets = std::array{
            vk::Offset3D{0, 0, 0},
            vk::Offset3D{
                static_cast<int32_t>(extent.width), static_cast<int32_t>(extent.height), 1
            }
        }
    };

    const vk::BlitImageInfo2 blit_image_info2 = {
        .srcImage = image_,
        .srcImageLayout = vk::ImageLayout::eTransferSrcOptimal,
        .dstImage = dst_image,
        .dstImageLayout = vk::ImageLayout::eTransferDstOptimal,
        .regionCount = 1,
        .pRegions = &blit_region,
        .filter = vk::Filter::eLinear
    };

    command_buffer.blitImage2(blit_image_info2);
}
}
