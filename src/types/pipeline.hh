//
// Created by damiendidier on 1/20/26.
//

#pragma once
#include <filesystem>
#include <vulkan/vulkan_raii.hpp>

#include "utils/types.hh"

namespace vk_tutorial::vk_types
{
class Pipeline : NonCopyable
{
    vk::raii::Pipeline pipeline_{nullptr};
    vk::raii::PipelineLayout layout_{nullptr};

public:
    constexpr Pipeline() = default;

    Pipeline(vk::raii::Pipeline pipeline, vk::raii::PipelineLayout layout);

    Pipeline(Pipeline&& other) noexcept;

    Pipeline& operator=(Pipeline&& other) noexcept;

    void swap(Pipeline& other) noexcept;

    const vk::Pipeline& getPipeline() const;
    const vk::PipelineLayout& getLayout() const;

    static Pipeline CreateGraphicPipeline(const vk::raii::Device& device, const std::filesystem::path& shader_path,
                                          std::span<const vk::Format> color_attachments,
                                          vk::Format depth_attachment, const vk::PipelineLayout& pipeline_layout);
};
}
