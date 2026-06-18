#include "required_queue_family_indices.hh"

#include <vector>

namespace vk_tutorial
{
void RequiredQueueFamilyIndices::populate(const vk::raii::PhysicalDevice& physical_device,
                                          const vk::raii::SurfaceKHR& surface)
{
    const auto queue_family_properties = physical_device.getQueueFamilyProperties();
    graphics_queue_family = getFirstQueueIndex(queue_family_properties, vk::QueueFlagBits::eGraphics);
    present_queue_family = getPresentQueueIndex(queue_family_properties, physical_device, surface,
                                                graphics_queue_family);
}

bool RequiredQueueFamilyIndices::isComplete() const
{
    return graphics_queue_family.has_value() && present_queue_family.has_value();
}

std::optional<uint32_t> RequiredQueueFamilyIndices::getFirstQueueIndex(
    const std::vector<vk::QueueFamilyProperties>& queue_families, const vk::QueueFlags flags)
{
    for (uint32_t i = 0; i < queue_families.size(); i++)
    {
        if ((queue_families[i].queueFlags & flags) == flags)
            return std::make_optional(i);
    }

    return std::nullopt;
}

std::optional<uint32_t> RequiredQueueFamilyIndices::getPresentQueueIndex(
    const std::vector<vk::QueueFamilyProperties>& queue_families,
    const vk::raii::PhysicalDevice& physical_device,
    const vk::raii::SurfaceKHR& surface,
    const std::optional<uint32_t> queue_family_hint = std::nullopt)
{
    if (queue_family_hint.has_value() && physical_device.getSurfaceSupportKHR(queue_family_hint.value(), surface))
        return queue_family_hint;
    for (uint32_t i = 0; i < queue_families.size(); i++)
    {
        if (physical_device.getSurfaceSupportKHR(i, surface))
            return std::make_optional(i);
    }

    return std::nullopt;
}
} // vk_tutorial
