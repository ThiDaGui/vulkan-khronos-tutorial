#include <cstdlib>
#include <exception>
#include <iostream>

#include "helloTriangleApplication.hh"

class GLFWWindowHandle {
    GLFWwindow *window = nullptr;
public:
    GLFWWindowHandle() {
        if (!glfwInit())
            throw std::runtime_error("failed to initialize GLFW!");

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        if (nullptr == (window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr)))
            throw std::runtime_error("failed to create GLFW window!");
    }

    ~GLFWWindowHandle() {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

[[nodiscard]] GLFWwindow * get_window() const {
        return window;
    }
};

int main(int argc, char *argv[]) {
    GLFWWindowHandle window_handle{};
    try {
        HelloTriangleApplicationCpp app{window_handle.get_window()};
        app.run();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
