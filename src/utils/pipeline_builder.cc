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
    shader_stages = {};
    color_blend_attachment_states = {};
    push_constant_ranges = {};
    vertex_input_state_create_info = vk::PipelineVertexInputStateCreateInfo{};
    input_assembly_state_create_info = vk::PipelineInputAssemblyStateCreateInfo{};
    viewport_state_create_info = vk::PipelineViewportStateCreateInfo{};
    dynamic_state_create_info = vk::PipelineDynamicStateCreateInfo{
        .dynamicStateCount = dynamic_states.size(), .pDynamicStates = dynamic_states.data()
    };
    rasterization_state_create_info = vk::PipelineRasterizationStateCreateInfo{.lineWidth = 1.0};
    multisample_state_create_info = vk::PipelineMultisampleStateCreateInfo{};
    color_blend_state_create_info = vk::PipelineColorBlendStateCreateInfo{};
    rendering_create_info = vk::PipelineRenderingCreateInfo{};
    layout = vk::PipelineLayout{};
}

constexpr const char *mainName(const vk::ShaderStageFlagBits stage)
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

PipelineBuilder& PipelineBuilder::addShaderStage(const vk::ShaderModule shader_module, const vk::ShaderStageFlagBits stage)
{

    this->shader_stages.push_back({.stage = stage, .module = shader_module, .pName = mainName(stage)});
    return *this;
}

PipelineBuilder& PipelineBuilder::setShaderStages(const vk::ShaderModule shader_module,
                                                  const vk::ShaderStageFlags stages)
{
    if (stages | vk::ShaderStageFlagBits::eVertex)
        shader_stages.push_back({
            .stage = vk::ShaderStageFlagBits::eVertex,
            .module = shader_module,
            .pName = "vertMain"
        });
    if (stages | vk::ShaderStageFlagBits::eFragment)
        shader_stages.push_back({
            .stage = vk::ShaderStageFlagBits::eFragment,
            .module = shader_module,
            .pName = "fragMain"
        });
    return *this;
}

PipelineBuilder& PipelineBuilder::setTopology(const vk::PrimitiveTopology topology)
{
    input_assembly_state_create_info.topology = topology;
    return *this;
}

PipelineBuilder& PipelineBuilder::setViewport(const uint32_t viewport_count, const uint32_t scissor_count)
{
    viewport_state_create_info.viewportCount = viewport_count;
    viewport_state_create_info.scissorCount = scissor_count;
    return *this;
}

PipelineBuilder& PipelineBuilder::setPolygonMode(vk::PolygonMode polygon_mode)
{
    rasterization_state_create_info.polygonMode = polygon_mode;
    return *this;
}

PipelineBuilder& PipelineBuilder::setRasterizationSample(const vk::SampleCountFlagBits samples)
{
    multisample_state_create_info.rasterizationSamples = samples;
    return *this;
}

PipelineBuilder& PipelineBuilder::addColorBlendAttachment(
    const vk::PipelineColorBlendAttachmentState& color_blend_attachment_state)
{
    color_blend_attachment_states.push_back(color_blend_attachment_state);
    return *this;
}

PipelineBuilder& PipelineBuilder::addPushConstantRange(const vk::PushConstantRange& push_constant_range)
{
    push_constant_ranges.push_back(push_constant_range);
    return *this;
}

PipelineBuilder& PipelineBuilder::setPipelineLayout(const vk::PipelineLayout& pipeline_layout)
{
    this->layout = pipeline_layout;
    return *this;
}

PipelineBuilder& PipelineBuilder::setColorAttachment(std::span<const vk::Format> color_attachments)
{
    rendering_create_info.setColorAttachmentFormats(color_attachments);
    return *this;
}

vk_types::Pipeline PipelineBuilder::buildGraphics(const vk::raii::Device& device)
{
    color_blend_state_create_info.setAttachments(color_blend_attachment_states);
    const vk::GraphicsPipelineCreateInfo create_info = {
        .pNext = rendering_create_info,
        .stageCount = static_cast<uint32_t>(shader_stages.size()),
        .pStages = shader_stages.data(),
        .pVertexInputState = &vertex_input_state_create_info,
        .pInputAssemblyState = &input_assembly_state_create_info,
        .pViewportState = &viewport_state_create_info,
        .pRasterizationState = &rasterization_state_create_info,
        .pMultisampleState = &multisample_state_create_info,
        .pColorBlendState = &color_blend_state_create_info,
        .pDynamicState = &dynamic_state_create_info,
        .layout = layout,
        .renderPass = nullptr,
    };

    return vk_types::Pipeline{vk::raii::Pipeline{device, nullptr, create_info}, vk::raii::PipelineLayout(device, layout)};
}
}
