//
// Created by damiendidier on 05/03/2026.
//

#include "descriptor_set_layout_builder.hh"

#include <vulkan/vulkan_raii.hpp>

namespace vk_tutorial
{
DescriptorSetLayoutBuilder& DescriptorSetLayoutBuilder::AddBinding(const std::uint32_t binding,
                                                                   const vk::DescriptorType type,
                                                                   const std::uint32_t count,
                                                                   const vk::ShaderStageFlags stages)
{
    bindings.emplace_back(binding, type, count, stages);
    return *this;
}

vk::raii::DescriptorSetLayout DescriptorSetLayoutBuilder::Build(const vk::raii::Device& device) const
{
    vk::DescriptorSetLayoutCreateInfo create_info = {
        .bindingCount = static_cast<uint32_t>(bindings.size()),
        .pBindings = bindings.data(),
    };

    return {device, create_info};
}
}
