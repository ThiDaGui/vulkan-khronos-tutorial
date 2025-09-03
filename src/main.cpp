#include <cstdlib>
#include <exception>
#include <iostream>

#include "helloTriangleApplication.hh"

class GLFWRaii {
public:
    GLFWRaii() {
        if (!glfwInit())
        throw std::runtime_error("failed to initialize GLFW!");
    }

    ~GLFWRaii() {
        glfwTerminate();
    }
};

int main(int argc, char *argv[]) {
    GLFWRaii glfw_raii{};

    try {
        HelloTriangleApplicationCpp app{};
        app.run();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
