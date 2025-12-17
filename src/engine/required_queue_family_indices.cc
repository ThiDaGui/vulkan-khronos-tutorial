#include "required_queue_family_indices.hh"

namespace vk_tutorial {
void RequiredQueueFamilyIndices::Populate(const vk::raii::PhysicalDevice &physical_device, const vk::raii::SurfaceKHR &surface) {
    const auto queue_family_properties = physical_device.getQueueFamilyProperties();
    graphics_queue_family = get_first_queue_index(queue_family_properties, vk::QueueFlagBits::eGraphics);
    present_queue_family = get_present_queue_index(queue_family_properties, physical_device, surface);
}

bool RequiredQueueFamilyIndices::isComplete() const {
    return graphics_queue_family.has_value() && present_queue_family.has_value();
}

std::optional<uint32_t> RequiredQueueFamilyIndices::get_first_queue_index(
    const std::vector<vk::QueueFamilyProperties> &queue_families, const vk::QueueFlags flags) {
    for (uint32_t i = 0; i < queue_families.size(); i++) {
        if ((queue_families[i].queueFlags & flags) == flags)
            return std::make_optional(i);
    }

    return std::nullopt;
}

std::optional<uint32_t> RequiredQueueFamilyIndices::get_present_queue_index(
    const std::vector<vk::QueueFamilyProperties> &queue_families, const vk::raii::PhysicalDevice &physical_device,
    const vk::raii::SurfaceKHR &surface) {
    for (uint32_t i = 0; i < queue_families.size(); i++) {
        if (physical_device.getSurfaceSupportKHR(i, surface))
            return std::make_optional(i);
    }

    return std::nullopt;
}

} // vk_tutorial