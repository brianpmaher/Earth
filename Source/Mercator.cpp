#include "Mercator.hpp"

#include <glm/gtc/constants.hpp>

#include <cmath>

namespace Earth::Mercator
{
    glm::vec3 UVToPosition(const glm::vec2& uv, float radius)
    {
        const float PI = glm::pi<float>();

        // Map U to Longitude [-PI, PI]
        float longitude = (uv.x * 2.0f * PI) - PI;

        // Map V to Latitude
        // Web Mercator V goes from 0 (North) to 1 (South).
        // We invert it so 0 is South and 1 is North for the math.
        float mercatorY = PI * (1.0f - 2.0f * uv.y);
        float latitude = 2.0f * std::atan(std::exp(mercatorY)) - (PI / 2.0f);

        // Spherical to Cartesian
        float cosLat = std::cos(latitude);

        float x = radius * cosLat * std::cos(longitude);
        float y = radius * std::sin(latitude);
        float z = radius * cosLat * std::sin(longitude);

        return glm::vec3(x, y, z);
    }

    Mesh GenerateSphereMesh(int resolution)
    {
        Mesh mesh;

        // Generate Vertices
        for (int y = 0; y <= resolution; y++)
        {
            for (int x = 0; x <= resolution; x++)
            {
                glm::vec2 uv;
                uv.x = (float)x / (float)resolution;
                uv.y = (float)y / (float)resolution;

                Vertex v;
                v.UV = uv;
                v.Position = UVToPosition(uv, 1.0f);

                mesh.Vertices.push_back(v);
            }
        }

        // Generate Indices
        for (int y = 0; y < resolution; y++)
        {
            for (int x = 0; x < resolution; x++)
            {
                uint32_t topLeft = y * (resolution + 1) + x;
                uint32_t topRight = topLeft + 1;
                uint32_t bottomLeft = (y + 1) * (resolution + 1) + x;
                uint32_t bottomRight = bottomLeft + 1;

                // Triangle 1
                mesh.Indices.push_back(topLeft);
                mesh.Indices.push_back(topRight);
                mesh.Indices.push_back(bottomLeft);

                // Triangle 2
                mesh.Indices.push_back(topRight);
                mesh.Indices.push_back(bottomRight);
                mesh.Indices.push_back(bottomLeft);
            }
        }

        return mesh;
    }
}
