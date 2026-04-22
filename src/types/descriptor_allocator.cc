#include "descriptor_allocator.hh"

namespace vk_tutorial::vk_types
{
DescriptorAllocator::DescriptorAllocator(const vk::raii::Device& device, const uint32_t max_sets,
                                         const std::span<const DescriptorEntry> entries)
{
    std::vector<vk::DescriptorPoolSize> pool_sizes{entries.size()};
    for (size_t i = 0; i < entries.size(); i++)
    {
        pool_sizes[i].type = entries[i].type;
        pool_sizes[i].descriptorCount = max_sets * entries[i].count;
    }

    const vk::DescriptorPoolCreateInfo create_info = {
        .maxSets = max_sets,
        .poolSizeCount = static_cast<uint32_t>(pool_sizes.size()),
        .pPoolSizes = pool_sizes.data(),
    };
    pool = vk::raii::DescriptorPool{device, create_info};
}

DescriptorAllocator::~DescriptorAllocator() = default;

DescriptorAllocator::DescriptorAllocator(DescriptorAllocator&& other) noexcept
{
    swap(other);
}

DescriptorAllocator& DescriptorAllocator::operator=(DescriptorAllocator&& other) noexcept
{
    swap(other);
    return *this;
}

void DescriptorAllocator::swap(DescriptorAllocator& other) noexcept
{
    if (this == &other)
        return;
    std::swap(pool, other.pool);
}

void DescriptorAllocator::reset() const
{
    pool.reset();
}

vk::DescriptorSet DescriptorAllocator::allocate(const vk::raii::Device& device,
                                                const vk::DescriptorSetLayout layout) const
{
    const vk::DescriptorSetAllocateInfo allocate_info = {
        .descriptorPool = pool,
        .descriptorSetCount = 1,
        .pSetLayouts = &layout,
    };
    return (*device).allocateDescriptorSets(allocate_info, *device.getDispatcher()).front();
}

std::vector<vk::raii::DescriptorSet> DescriptorAllocator::allocate(const vk::raii::Device& device,
                                                                   const vk::DescriptorSetLayout layout,
                                                                   const uint32_t count) const
{
    const vk::DescriptorSetAllocateInfo allocate_info = {
        .descriptorPool = pool,
        .descriptorSetCount = count,
        .pSetLayouts = &layout,
    };
    return vk::raii::DescriptorSets(device, allocate_info);
}
}
