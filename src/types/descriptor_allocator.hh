#pragma once
#include <vulkan/vulkan_raii.hpp>

#include "utils/types.hh"

namespace vk_tutorial:: vk_types
{
class DescriptorAllocator : NonMovable
{
    vk::raii::DescriptorPool pool{nullptr};

public:
    struct DescriptorEntry
    {
        vk::DescriptorType type;
        uint32_t count;
    };

    DescriptorAllocator() = default;

    DescriptorAllocator(const vk::raii::Device& device, uint32_t max_sets, std::span<const DescriptorEntry> entries);

    ~DescriptorAllocator();

    DescriptorAllocator(DescriptorAllocator&& other) noexcept;

    DescriptorAllocator& operator=(DescriptorAllocator&& other) noexcept;

    void swap(DescriptorAllocator& other);

    void reset() const;

    vk::raii::DescriptorSet allocate(const vk::raii::Device& device, vk::DescriptorSetLayout layout) const;

    std::vector<vk::raii::DescriptorSet> allocate(const vk::raii::Device& device, vk::DescriptorSetLayout layout,
                                                  uint32_t count) const;
};
} // vk_tutorial::vk_types
