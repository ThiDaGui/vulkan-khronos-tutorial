#include "pipeline_builder.hh"

std::vector<char> readShader(const std::filesystem::path& file_path);

namespace vk_tutorial
{
PipelineBuilder::PipelineBuilder()
{
    reset();
}

void PipelineBuilder::reset()
{
    shader_stages_ = {};
    color_blend_attachment_states_ = {};
    push_constant_ranges_ = {};
    vertex_input_state_create_info_ = vk::PipelineVertexInputStateCreateInfo{};
    input_assembly_state_create_info_ = vk::PipelineInputAssemblyStateCreateInfo{};
    viewport_state_create_info_ = vk::PipelineViewportStateCreateInfo{};
    dynamic_state_create_info_ = vk::PipelineDynamicStateCreateInfo{
        .dynamicStateCount = DYNAMIC_STATES.size(), .pDynamicStates = DYNAMIC_STATES.data()
    };
    rasterization_state_create_info_ = vk::PipelineRasterizationStateCreateInfo{.lineWidth = 1.0};
    multisample_state_create_info_ = vk::PipelineMultisampleStateCreateInfo{};
    color_blend_state_create_info_ = vk::PipelineColorBlendStateCreateInfo{};
    rendering_create_info_ = vk::PipelineRenderingCreateInfo{};
    layout_ = vk::PipelineLayout{};
}

constexpr const char* mainName(const vk::ShaderStageFlagBits stage)
{
    switch (stage)
    {
    case vk::ShaderStageFlagBits::eVertex:
        return "vertName";
    case vk::ShaderStageFlagBits::eFragment:
        return "fragName";
    default:
        return nullptr;
    }
}

PipelineBuilder& PipelineBuilder::addShaderStage(const vk::ShaderModule shader_module,
                                                 const vk::ShaderStageFlagBits stage)
{
    this->shader_stages_.push_back({.stage = stage, .module = shader_module, .pName = mainName(stage)});
    return *this;
}

PipelineBuilder& PipelineBuilder::setShaderStages(const vk::ShaderModule shader_module,
                                                  const vk::ShaderStageFlags stages)
{
    if (stages | vk::ShaderStageFlagBits::eVertex)
        shader_stages_.push_back({
            .stage = vk::ShaderStageFlagBits::eVertex,
            .module = shader_module,
            .pName = "vertMain"
        });
    if (stages | vk::ShaderStageFlagBits::eFragment)
        shader_stages_.push_back({
            .stage = vk::ShaderStageFlagBits::eFragment,
            .module = shader_module,
            .pName = "fragMain"
        });
    return *this;
}

PipelineBuilder& PipelineBuilder::setTopology(const vk::PrimitiveTopology topology)
{
    input_assembly_state_create_info_.topology = topology;
    return *this;
}

PipelineBuilder& PipelineBuilder::setViewport(const uint32_t viewport_count, const uint32_t scissor_count)
{
    viewport_state_create_info_.viewportCount = viewport_count;
    viewport_state_create_info_.scissorCount = scissor_count;
    return *this;
}

PipelineBuilder& PipelineBuilder::setPolygonMode(const vk::PolygonMode polygon_mode)
{
    rasterization_state_create_info_.polygonMode = polygon_mode;
    return *this;
}

PipelineBuilder& PipelineBuilder::setRasterizationSample(const vk::SampleCountFlagBits samples)
{
    multisample_state_create_info_.rasterizationSamples = samples;
    return *this;
}

PipelineBuilder& PipelineBuilder::addColorBlendAttachment(
    const vk::PipelineColorBlendAttachmentState& color_blend_attachment_state)
{
    color_blend_attachment_states_.push_back(color_blend_attachment_state);
    return *this;
}

PipelineBuilder& PipelineBuilder::addPushConstantRange(const vk::PushConstantRange& push_constant_range)
{
    push_constant_ranges_.push_back(push_constant_range);
    return *this;
}

PipelineBuilder& PipelineBuilder::setPipelineLayout(const vk::PipelineLayout& pipeline_layout)
{
    this->layout_ = pipeline_layout;
    return *this;
}

PipelineBuilder& PipelineBuilder::setColorAttachment(std::span<const vk::Format> color_attachments)
{
    rendering_create_info_.setColorAttachmentFormats(color_attachments);
    return *this;
}

PipelineBuilder& PipelineBuilder::setDepthAttachment(const vk::Format depth_stencil_format)
{
    rendering_create_info_.setDepthAttachmentFormat(depth_stencil_format);
    return *this;
}

PipelineBuilder& PipelineBuilder::setDepthTest(const bool depth_test, const bool depth_write,
                                               const vk::CompareOp depth_compare_op)
{
    depth_stencil_state_create_info_.depthTestEnable = depth_test;
    depth_stencil_state_create_info_.depthWriteEnable = depth_write;
    depth_stencil_state_create_info_.depthCompareOp = depth_compare_op;
    return *this;
}

vk_types::Pipeline PipelineBuilder::buildGraphics(const vk::raii::Device& device)
{
    color_blend_state_create_info_.setAttachments(color_blend_attachment_states_);
    const vk::GraphicsPipelineCreateInfo create_info = {
        .pNext = rendering_create_info_,
        .stageCount = static_cast<uint32_t>(shader_stages_.size()),
        .pStages = shader_stages_.data(),
        .pVertexInputState = &vertex_input_state_create_info_,
        .pInputAssemblyState = &input_assembly_state_create_info_,
        .pViewportState = &viewport_state_create_info_,
        .pRasterizationState = &rasterization_state_create_info_,
        .pMultisampleState = &multisample_state_create_info_,
        .pDepthStencilState = &depth_stencil_state_create_info_,
        .pColorBlendState = &color_blend_state_create_info_,
        .pDynamicState = &dynamic_state_create_info_,
        .layout = layout_,
        .renderPass = nullptr,
    };

    return vk_types::Pipeline{
        vk::raii::Pipeline{device, nullptr, create_info}, vk::raii::PipelineLayout(device, layout_)
    };
}
}
