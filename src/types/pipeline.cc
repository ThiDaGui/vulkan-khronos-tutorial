//
// Created by damiendidier on 1/20/26.
//

#include "pipeline.hh"

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "vulkanUtils.hh"

namespace vk_tutorial::vk_types
{
Pipeline::Pipeline(vk::raii::Pipeline pipeline, vk::raii::PipelineLayout layout)
    : pipeline_(std::move(pipeline))
    , layout_(std::move(layout))
{}

Pipeline Pipeline::CreateGraphicPipeline(const vk::raii::Device& device,
                                         const std::filesystem::path& shader_path,
                                         const std::span<const vk::Format> color_attachments)
{
    std::vector buffer{readShader(shader_path)};
    const vk::ShaderModuleCreateInfo shader_module_create_info = {
        .codeSize = buffer.size() * sizeof(char),
        .pCode = reinterpret_cast<uint32_t*>(buffer.data()),
    };

    const vk::raii::ShaderModule module{device, shader_module_create_info};

    const std::array shader_stage_create_infos = {
        vk::PipelineShaderStageCreateInfo{
            .stage = vk::ShaderStageFlagBits::eVertex,
            .module = module,
            .pName = "vertMain",
        },
        vk::PipelineShaderStageCreateInfo{
            .stage = vk::ShaderStageFlagBits::eFragment,
            .module = module,
            .pName = "fragMain",
        }
    };

    static constexpr vk::PipelineVertexInputStateCreateInfo vertex_input_state_create_info = {
        .vertexBindingDescriptionCount = 0,
        .vertexAttributeDescriptionCount = 0,
    };

    static constexpr vk::PipelineInputAssemblyStateCreateInfo input_assembly_state_create_info = {
        .topology = vk::PrimitiveTopology::eTriangleList
    };

    static constexpr vk::PipelineViewportStateCreateInfo viewport_state_create_info = {
        .viewportCount = 1,
        .scissorCount = 1,
    };
    static constexpr std::array dynamic_states = {
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor,
    };

    static constexpr vk::PipelineRasterizationStateCreateInfo rasterization_state_create_info = {
        .polygonMode = vk::PolygonMode::eFill
    };

    static constexpr vk::PipelineMultisampleStateCreateInfo multisample_state_create_info = {};

    static constexpr vk::PipelineColorBlendAttachmentState color_blend_attachment_state = {
        .blendEnable = vk::False,
        .colorWriteMask = vk::ColorComponentFlagBits::eR |
                          vk::ColorComponentFlagBits::eG |
                          vk::ColorComponentFlagBits::eB |
                          vk::ColorComponentFlagBits::eA,
    };

    static constexpr vk::PipelineColorBlendStateCreateInfo color_blend_state_create_info = {
        .logicOpEnable = vk::False,
        .attachmentCount = 1,
        .pAttachments = &color_blend_attachment_state
    };

    static constexpr vk::PipelineDynamicStateCreateInfo dynamic_state_create_info = {
        .dynamicStateCount = static_cast<uint32_t>(dynamic_states.size()),
        .pDynamicStates = dynamic_states.data()
    };

    const vk::PipelineRenderingCreateInfo rendering_create_info = {
        .colorAttachmentCount = static_cast<uint32_t>(color_attachments.size()),
        .pColorAttachmentFormats = color_attachments.data(),
    };

    vk::PipelineLayoutCreateInfo pipeline_layout_create_info = {
        .setLayoutCount = 0,
        .pushConstantRangeCount = 0,
    };

    vk::raii::PipelineLayout layout{device, pipeline_layout_create_info};

    const vk::GraphicsPipelineCreateInfo pipeline_create_info = {
        .pNext = rendering_create_info,
        .stageCount = shader_stage_create_infos.size(),
        .pStages = shader_stage_create_infos.data(),
        //        .pVertexInputState = &vertex_input_state_create_info,
        //        .pInputAssemblyState = &input_assembly_state_create_info,
        .pViewportState = &viewport_state_create_info,
        .pRasterizationState = &rasterization_state_create_info,
        .pMultisampleState = &multisample_state_create_info,
        .pColorBlendState = &color_blend_state_create_info,
        .pDynamicState = &dynamic_state_create_info,
        .layout = layout,
        .renderPass = nullptr,
    };

    return Pipeline{vk::raii::Pipeline{device, nullptr, pipeline_create_info}, std::move(layout)};
}
}
