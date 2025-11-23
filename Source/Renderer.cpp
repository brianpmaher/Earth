#include "Renderer.hpp"

#include <vector>

namespace Earth
{
    Renderer::Renderer() : m_Shader{"Assets/Shaders/Earth.vert.glsl", "Assets/Shaders/Earth.frag.glsl"}
    {
    }

    Renderer::~Renderer()
    {
        if (m_VAO)
            glDeleteVertexArrays(1, &m_VAO);
        if (m_VBO)
            glDeleteBuffers(1, &m_VBO);
        if (m_EBO)
            glDeleteBuffers(1, &m_EBO);
    }

    void Renderer::UploadMesh(const Mesh& mesh)
    {
        if (m_VAO == 0)
            glGenVertexArrays(1, &m_VAO);
        if (m_VBO == 0)
            glGenBuffers(1, &m_VBO);
        if (m_EBO == 0)
            glGenBuffers(1, &m_EBO);

        glBindVertexArray(m_VAO);

        // VBO
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, mesh.Vertices.size() * sizeof(Vertex), mesh.Vertices.data(), GL_STATIC_DRAW);

        // EBO
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.Indices.size() * sizeof(uint32_t), mesh.Indices.data(),
                     GL_STATIC_DRAW);

        // Attributes
        // 0: Position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Position));

        // 1: UV
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, UV));

        glBindVertexArray(0);

        m_IndexCount = (GLsizei)mesh.Indices.size();
    }

    void Renderer::DrawTile(const glm::mat4& viewProjection, int x, int y, int z, bool showGrid)
    {
        m_Shader.Bind();

        GLint loc = glGetUniformLocation(m_Shader.GetRendererID(), "u_ViewProjection");
        glUniformMatrix4fv(loc, 1, GL_FALSE, &viewProjection[0][0]);

        m_Shader.SetInt("u_TileX", x);
        m_Shader.SetInt("u_TileY", y);
        m_Shader.SetInt("u_TileZ", z);
        m_Shader.SetBool("u_ShowGrid", showGrid);
        m_Shader.SetInt("u_ColorTexture", 0);
        m_Shader.SetInt("u_ElevationTexture", 1);

        glBindVertexArray(m_VAO);
        glDrawElements(GL_TRIANGLES, m_IndexCount, GL_UNSIGNED_INT, 0);
    }
}
