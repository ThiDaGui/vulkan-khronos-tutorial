#include "pipeline.hh"

#include <fstream>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "utils/pipeline_builder.hh"

std::vector<char> readShader(const std::filesystem::path& file_path)
{
    std::ifstream file{file_path, std::ios::ate | std::ios::binary};

    if (!file.is_open())
        throw std::runtime_error("Failed to open file " + file_path.string());

    std::vector<char> buffer(file.tellg());

    file.seekg(std::ios::beg);
    file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));

    file.close();
    return buffer;
}


namespace vk_tutorial::vk_types
{
Pipeline::Pipeline(vk::raii::Pipeline pipeline, vk::raii::PipelineLayout layout)
    : pipeline_(std::move(pipeline))
    , layout_(std::move(layout))
{}

Pipeline::Pipeline(Pipeline&& other) noexcept
{
    swap(other);
}

Pipeline& Pipeline::operator=(Pipeline&& other) noexcept
{
    swap(other);
    return *this;
}

void Pipeline::swap(Pipeline& other) noexcept

{
    if (this == &other)
        return;
    std::swap(pipeline_, other.pipeline_);
    std::swap(layout_, other.layout_);
}

const vk::Pipeline& Pipeline::getPipeline() const
{
    return *pipeline_;
}

const vk::PipelineLayout& Pipeline::getLayout() const
{
    return *layout_;
}

Pipeline Pipeline::CreateGraphicPipeline(const vk::raii::Device& device,
                                         const std::filesystem::path& shader_path,
                                         const std::span<const vk::Format> color_attachments)
{
    PipelineBuilder builder{};
    std::vector buffer{readShader(shader_path)};
    const vk::ShaderModuleCreateInfo shader_module_create_info = {
        .codeSize = buffer.size() * sizeof(char),
        .pCode = reinterpret_cast<uint32_t*>(buffer.data()),
    };

    const vk::raii::ShaderModule module{device, shader_module_create_info};
    builder.setShaderStages(module, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment)
           .setTopology(vk::PrimitiveTopology::eTriangleList)
           .setViewport(1, 1)
           .setPolygonMode(vk::PolygonMode::eFill)
           .addColorBlendAttachment({
               .blendEnable = vk::False,
               .colorWriteMask = vk::ColorComponentFlagBits::eR |
                                 vk::ColorComponentFlagBits::eG |
                                 vk::ColorComponentFlagBits::eB |
                                 vk::ColorComponentFlagBits::eA
           })
           .setColorAttachment(color_attachments);
    return builder.buildGraphics(device);
}
}
