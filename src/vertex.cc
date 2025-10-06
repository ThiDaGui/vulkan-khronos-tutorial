//
// Created by damiendidier on 16/09/2025.
//

#include "vertex.hh"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

vk::VertexInputBindingDescription Vertex::getBindingDescription()
{
    return vk::VertexInputBindingDescription {
        0, sizeof(Vertex), vk::VertexInputRate::eVertex
    };
}

std::array<vk::VertexInputAttributeDescription, 3>
Vertex::getAttributeDescriptions()
{
    return {
        vk::VertexInputAttributeDescription {0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, position) },
        vk::VertexInputAttributeDescription {1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, color) },
        vk::VertexInputAttributeDescription {2, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, tex_coordinates) },
    };
}

std::size_t std::hash<Vertex>::operator()(const Vertex &key) const noexcept
{
    return ((std::hash<glm::vec3>{}(key.position) ^ (std::hash<glm::vec3>{}(key.color) << 1)) >> 1) ^
        (std::hash<glm::vec2>{}(key.tex_coordinates) << 1);
}
