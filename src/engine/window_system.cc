#include "window_system.hh"

#include <GLFW/glfw3.h>

namespace vk_tutorial
{
void framebufferResizeCallback(GLFWwindow* window, [[maybe_unused]] int width, [[maybe_unused]] int height)
{
    const auto window_system = static_cast<WindowSystem*>(glfwGetWindowUserPointer(window));
    window_system->resized = true;
}

WindowSystem::~WindowSystem()
{
    if (nullptr == window)
        return;
    glfwDestroyWindow(window);
    glfwTerminate();
}

void WindowSystem::init(const uint32_t width, const uint32_t height, const char* name)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(static_cast<int>(width), static_cast<int>(height), name, nullptr, nullptr);
    if (nullptr == window)
        throw std::runtime_error("Failed to create GLFW window!");
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    glfwSetWindowUserPointer(window, this);
}

std::vector<const char*> WindowSystem::getRequiredExtensions()
{
    uint32_t glfw_extension_count = 0;
    const auto glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
    return {glfw_extensions, glfw_extensions + glfw_extension_count};
}

vk::raii::SurfaceKHR WindowSystem::createSurface(const vk::raii::Instance& instance) const
{
    VkSurfaceKHR vk_surface;
    auto err = glfwCreateWindowSurface(*instance, window, nullptr, &vk_surface);
    if (static_cast<VkResult>(vk::Result::eSuccess) != err)
        throw std::runtime_error("failed to create window surface!");
    return {instance, vk_surface};
}

vk::Extent2D WindowSystem::getExtent() const
{
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    while (0 == width && 0 == height)
    {
        glfwWaitEvents();
        glfwGetWindowSize(window, &width, &height);
    }
    return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
}

bool WindowSystem::shouldClose() const
{
    glfwPollEvents();
    return glfwWindowShouldClose(window);
}
}
