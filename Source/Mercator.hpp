#pragma once

#include "Mesh.hpp"

#include <glm/glm.hpp>

namespace Earth::Mercator
{
    // Converts a Web Mercator UV (0-1) to a 3D position on a sphere
    glm::vec3 UVToPosition(const glm::vec2& uv, float radius = 1.0f);
    glm::vec2 PositionToUV(const glm::vec3& position);

    Mesh GeneratePlaneMesh(int resolution);
}
