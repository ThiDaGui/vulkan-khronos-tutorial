import vulkan_hpp;

#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>

void listRequiredInstanceExtensions(
    const std::vector<const char *> &extensions) {
    std::cout << "required extensions:\n";

    for (const auto extension : extensions) {
        std::cout << '\t' << extension << std::endl;
    }
}

std::vector<const char *> getRequiredInstanceExtensions() {
    uint32_t glfwExtensionCount = 0;
    auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char *> extensions(glfwExtensions,
                                         glfwExtensions + glfwExtensionCount);

#ifndef NDEBUG
    extensions.push_back(vk::EXTDebugUtilsExtensionName);
    listRequiredInstanceExtensions(extensions);
#endif

    return extensions;
}
