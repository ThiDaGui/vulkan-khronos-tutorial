//
// Created by damiendidier on 16/09/2025.
//

#include "vertex.hh"

vk::VertexInputBindingDescription Vertex::getBindingDescription()
{
    return vk::VertexInputBindingDescription {
        0, sizeof(Vertex), vk::VertexInputRate::eVertex
    };
}

std::array<vk::VertexInputAttributeDescription, 2> Vertex::
getAttributeDescriptions()
{
    return {
        vk::VertexInputAttributeDescription {0, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, position) },
        vk::VertexInputAttributeDescription {1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, color) },
    };
}