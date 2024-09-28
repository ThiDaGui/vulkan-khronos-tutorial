#include <GLFW/glfw3.h>
#include <iostream>
#include <set>
#include <vector>
#include <vulkan/vulkan.hpp>

bool checkValidationLayerSupport(std::vector<const char *> validationLayers) {
    std::vector<vk::LayerProperties> availableLayers =
        vk::enumerateInstanceLayerProperties();

    std::set<std::string> requiredLayers{ validationLayers.begin(),
                                          validationLayers.end() };

    for (const auto &layerProperty : availableLayers) {
        requiredLayers.erase(layerProperty.layerName);
    }
    return requiredLayers.empty();
}

void listRequiredInstanceExtensions(
    const std::vector<const char *> &extensions) {
    std::cout << "required extensions:\n";

    for (const auto extension : extensions) {
        std::cout << '\t' << extension << std::endl;
    }
}

std::vector<const char *> getRequiredInstanceExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char *> extensions(glfwExtensions,
                                         glfwExtensions + glfwExtensionCount);

#ifndef NDEBUG
    extensions.push_back(vk::EXTDebugUtilsExtensionName);
    listRequiredInstanceExtensions(extensions);
#endif

    return extensions;
}

