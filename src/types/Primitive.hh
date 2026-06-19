#pragma once

#include "typed_buffer.hh"

#include <glm/vec3.hpp>

namespace vk_tutorial
{
struct Core;
}

namespace vk_tutorial::vk_types
{
struct Vertex
{
    glm::vec3 position;
    glm::vec3 color;
};

struct Primitive
{
    TypedBuffer<Vertex> vertex_buffer;
    vk::DeviceAddress vertex_buffer_address;

    void init(const Core& core, std::span<const Vertex> vertices);
};
} // vk_tutorial::vk_types
