#pragma once

#include <glm/glm.hpp>

#include <vector>

namespace Earth
{
    struct Vertex
    {
        glm::vec3 Position;
        glm::vec2 UV;
    };

    struct Mesh
    {
        std::vector<Vertex> Vertices;
        std::vector<uint32_t> Indices;
    };
}
