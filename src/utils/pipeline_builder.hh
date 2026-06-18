#pragma once

#include "types/pipeline.hh"

namespace vk_tutorial
{
class PipelineBuilder
{
    static constexpr std::array DYNAMIC_STATES{vk::DynamicState::eViewport, vk::DynamicState::eScissor};

    std::vector<vk::PipelineShaderStageCreateInfo> shader_stages_;

    std::vector<vk::PipelineColorBlendAttachmentState> color_blend_attachment_states_;

    std::vector<vk::PushConstantRange> push_constant_ranges_;

    vk::PipelineVertexInputStateCreateInfo vertex_input_state_create_info_;

    vk::PipelineInputAssemblyStateCreateInfo input_assembly_state_create_info_;

    vk::PipelineViewportStateCreateInfo viewport_state_create_info_;

    vk::PipelineDynamicStateCreateInfo dynamic_state_create_info_;

    vk::PipelineRasterizationStateCreateInfo rasterization_state_create_info_;

    vk::PipelineMultisampleStateCreateInfo multisample_state_create_info_;

    vk::PipelineDepthStencilStateCreateInfo depth_stencil_state_create_info_;

    vk::PipelineColorBlendStateCreateInfo color_blend_state_create_info_;

    vk::PipelineRenderingCreateInfo rendering_create_info_;

    vk::PipelineLayout layout_;

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

    PipelineBuilder& setDepthAttachment(vk::Format depth_stencil_format);

    PipelineBuilder& setDepthTest(bool depth_test, bool depth_write, vk::CompareOp depth_compare_op);

    vk_types::Pipeline buildGraphics(const vk::raii::Device& device);
};
}
