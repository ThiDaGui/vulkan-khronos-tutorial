
#pragma once
#include <vector>
#include <vulkan/vulkan_raii.hpp>

struct GLFWwindow;

namespace vk_tutorial
{
struct WindowSystem {
    GLFWwindow *window{nullptr};
    bool resized{false};

    WindowSystem() = default;
    ~WindowSystem();

    WindowSystem(const WindowSystem &other) = delete;
    WindowSystem &operator=(const WindowSystem &other) = delete;
    WindowSystem(WindowSystem &&other) = delete;
    WindowSystem &operator=(WindowSystem &&other) = delete;

    void init(std::uint32_t width, std::uint32_t height, const char *name);

    static std::vector<const char *> getRequiredExtensions();

    vk::raii::SurfaceKHR createSurface(const vk::raii::Instance& instance) const;

    [[nodiscard]] vk::Extent2D getExtent() const;

    [[nodiscard]] bool shouldClose() const;
};
}
