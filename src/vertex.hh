//
// Created by damiendidier on 16/09/2025.
//

#pragma once

#include <functional>

#include <glm/glm.hpp>

#include <vulkan/vulkan.hpp>

struct Vertex {
    glm::vec3 position;
    glm::vec3 color;
    glm::vec2 tex_coordinates;

    static vk::VertexInputBindingDescription getBindingDescription();
    static std::array<vk::VertexInputAttributeDescription, 3> getAttributeDescriptions();

    bool operator==(const Vertex &) const = default;
};

template<>
struct std::hash<Vertex>{
    std::size_t operator()(const Vertex &key) const noexcept;
};