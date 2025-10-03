//
// Created by damiendidier on 16/09/2025.
//

#pragma once

import vulkan_hpp;

#include <glm/glm.hpp>

struct Vertex {
    glm::vec2 position;
    glm::vec3 color;
    glm::vec2 tex_coordinates;

    static vk::VertexInputBindingDescription getBindingDescription();
    static std::array<vk::VertexInputAttributeDescription, 3> getAttributeDescriptions();
};