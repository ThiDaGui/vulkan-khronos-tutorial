
#pragma once
#include <vector>
#include <vulkan/vulkan_raii.hpp>

#include "utils/types.hh"

// ReSharper disable once CppInconsistentNaming
struct GLFWwindow;

namespace vk_tutorial
{
struct WindowSystem final : NonMovable
{
    GLFWwindow* window{nullptr};
    bool resized{false};

    WindowSystem() = default;
    ~WindowSystem();

    void init(uint32_t width, uint32_t height, const char* name);

    static std::vector<const char*> getRequiredExtensions();

    [[nodiscard]] vk::raii::SurfaceKHR createSurface(const vk::raii::Instance& instance) const;

    [[nodiscard]] vk::Extent2D getExtent() const;

    [[nodiscard]] bool shouldClose() const;
};
}
