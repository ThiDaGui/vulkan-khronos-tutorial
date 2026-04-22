#pragma once

#include "types/pipeline.hh"

namespace vk_tutorial
{
class PipelineBuilder
{
    static constexpr std::array dynamic_states{vk::DynamicState::eViewport, vk::DynamicState::eScissor};

    std::vector<vk::PipelineShaderStageCreateInfo> shader_stages;

    std::vector<vk::PipelineColorBlendAttachmentState> color_blend_attachment_states;

    std::vector<vk::PushConstantRange> push_constant_ranges;

    vk::PipelineVertexInputStateCreateInfo vertex_input_state_create_info;

    vk::PipelineInputAssemblyStateCreateInfo input_assembly_state_create_info;

    vk::PipelineViewportStateCreateInfo viewport_state_create_info;

    vk::PipelineDynamicStateCreateInfo dynamic_state_create_info;

    vk::PipelineRasterizationStateCreateInfo rasterization_state_create_info;

    vk::PipelineMultisampleStateCreateInfo multisample_state_create_info;

    vk::PipelineColorBlendStateCreateInfo color_blend_state_create_info;

    vk::PipelineRenderingCreateInfo rendering_create_info;

    vk::PipelineLayout layout;

public:
    PipelineBuilder();

    void reset();

    PipelineBuilder& addShaderStage(vk::ShaderModule shader_module, vk::ShaderStageFlagBits stage);

    PipelineBuilder& setShaderStages(vk::ShaderModule shader_module, vk::ShaderStageFlags stages);

    PipelineBuilder& setTopology(vk::PrimitiveTopology topology);

    PipelineBuilder& setViewport(uint32_t viewport_count, uint32_t scissor_count);

    PipelineBuilder& setPolygonMode(vk::PolygonMode polygon_mode);

    PipelineBuilder& setRasterizationSample(vk::SampleCountFlagBits samples);

    PipelineBuilder& addColorBlendAttachment(const vk::PipelineColorBlendAttachmentState& color_blend_attachment_state);

    PipelineBuilder& addPushConstantRange(const vk::PushConstantRange& push_constant_range);

    PipelineBuilder& setPipelineLayout(const vk::PipelineLayout& pipeline_layout);

    PipelineBuilder& setColorAttachment(std::span<const vk::Format> color_attachments);

    vk_types::Pipeline buildGraphics(const vk::raii::Device& device);
};
}
