#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan_raii.hpp>

#include "utils/types.hh"

namespace vk_tutorial::vk_types
{
class Image final : NonCopyable
{
public:
    enum class Usage
    {
        eTexture,
        eColorAttachment,
        eDepth,
        eDepthStencil,
    };

    vk::Extent3D image_extent{};
    vk::Format image_format{};
    vk::raii::Image image{nullptr};
    vk::raii::ImageView image_view{nullptr};
    VmaAllocation image_memory{nullptr};
    VmaAllocator vma_allocator{};

    Image() = default;

    Image(const vk::raii::Device& device,
          VmaAllocator vma_allocator,
          vk::Format format,
          vk::Extent2D extent_2d,
          vk::SampleCountFlagBits sample_count,
          Usage image_usage,
          vk::ImageAspectFlags image_aspect);

    ~Image();

    Image(Image&& other) noexcept;

    Image& operator=(Image&& other) noexcept;

    void swap(Image& other) noexcept;

    void transition(const vk::raii::CommandBuffer& command_buffer,
                    vk::PipelineStageFlagBits2 src_stage_mask,
                    vk::AccessFlagBits2 src_access_mask,
                    vk::PipelineStageFlagBits2 dst_stage_mask,
                    vk::AccessFlagBits2 dst_access_mask,
                    vk::ImageLayout old_layout,
                    vk::ImageLayout new_layout,
                    vk::ImageAspectFlags aspect_mask) const;

    static void transition(const vk::raii::CommandBuffer& command_buffer,
                           vk::Image image,
                           vk::PipelineStageFlagBits2 src_stage_mask,
                           vk::AccessFlagBits2 src_access_mask,
                           vk::PipelineStageFlagBits2 dst_stage_mask,
                           vk::AccessFlagBits2 dst_access_mask,
                           vk::ImageLayout old_layout,
                           vk::ImageLayout new_layout,
                           vk::ImageAspectFlags aspect_mask);

    void copy(const vk::raii::CommandBuffer& command_buffer,
              const Image& dst_image) const;

    void copy(const vk::raii::CommandBuffer& command_buffer, vk::Image dst_image, vk::Extent2D extent) const;
};
}
