//
// Created by damiendidier on 05/03/2026.
//

#include "descriptor_set_layout_builder.hh"

#include <vulkan/vulkan_raii.hpp>

namespace vk_tutorial
{
DescriptorSetLayoutBuilder& DescriptorSetLayoutBuilder::addBinding(const std::uint32_t binding,
                                                                   const vk::DescriptorType type,
                                                                   const std::uint32_t count,
                                                                   const vk::ShaderStageFlags stages)
{
    bindings_.emplace_back(binding, type, count, stages);
    return *this;
}

vk::raii::DescriptorSetLayout DescriptorSetLayoutBuilder::build(const vk::raii::Device& device) const
{
    vk::DescriptorSetLayoutCreateInfo create_info = {
        .bindingCount = static_cast<uint32_t>(bindings_.size()),
        .pBindings = bindings_.data(),
    };

    return {device, create_info};
}
}
