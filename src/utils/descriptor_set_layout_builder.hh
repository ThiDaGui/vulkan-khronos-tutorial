//
// Created by damiendidier on 05/03/2026.
//

#pragma once
#include <vector>

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

namespace vk::raii
{
class DescriptorSetLayout;
}

namespace vk_tutorial
{
class DescriptorSetLayoutBuilder
{
    std::vector<vk::DescriptorSetLayoutBinding> bindings_{};

public:
    DescriptorSetLayoutBuilder& addBinding(uint32_t binding, vk::DescriptorType type, uint32_t count,
                                           vk::ShaderStageFlags stages);

    [[nodiscard]] vk::raii::DescriptorSetLayout build(const vk::raii::Device& device) const;
};
}
