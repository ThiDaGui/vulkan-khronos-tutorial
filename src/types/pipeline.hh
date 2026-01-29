//
// Created by damiendidier on 1/20/26.
//

#pragma once
#include <filesystem>
#include <vulkan/vulkan_raii.hpp>

namespace vk_tutorial::vk_types
{
class Pipeline
{
    vk::raii::Pipeline pipeline_;
    vk::raii::PipelineLayout layout_;

public:
    Pipeline(vk::raii::Pipeline pipeline, vk::raii::PipelineLayout layout);

    Pipeline(const Pipeline& other) = delete;

    Pipeline(Pipeline&& other) noexcept
        : pipeline_(std::move(other.pipeline_))
        , layout_(std::move(other.layout_))
    {}

    Pipeline& operator=(const Pipeline& other) = delete;

    Pipeline& operator=(Pipeline&& other) noexcept
    {
        if (this == &other)
            return *this;
        pipeline_ = std::move(other.pipeline_);
        layout_ = std::move(other.layout_);
        return *this;
    }

    static Pipeline CreateGraphicPipeline(const vk::raii::Device& device, const std::filesystem::path& shader_path,
                                   std::span<const vk::Format> color_attachments);
};
}
