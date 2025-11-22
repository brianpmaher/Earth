#include "Renderer.hpp"

#include <fstream>
#include <print>
#include <sstream>
#include <vector>

namespace
{
    std::string ReadFile(const std::string& filepath)
    {
        std::ifstream file(filepath);
        if (!file.is_open())
        {
            std::println("Failed to open file: {}", filepath);
            return "";
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    GLuint CompileShader(GLenum type, const std::string& source)
    {
        GLuint shader = glCreateShader(type);
        const char* src = source.c_str();
        glShaderSource(shader, 1, &src, NULL);
        glCompileShader(shader);

        GLint success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            char infoLog[512];
            glGetShaderInfoLog(shader, 512, NULL, infoLog);
            std::println("Shader Compile Error: {}", infoLog);
        }

        return shader;
    }

    GLuint CreateShader(const std::string& vertexSrc, const std::string& fragmentSrc)
    {
        GLuint vs = CompileShader(GL_VERTEX_SHADER, vertexSrc);
        GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fragmentSrc);

        GLuint program = glCreateProgram();
        glAttachShader(program, vs);
        glAttachShader(program, fs);
        glLinkProgram(program);

        GLint success;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success)
        {
            char infoLog[512];
            glGetProgramInfoLog(program, 512, NULL, infoLog);
            std::println("Shader Link Error: {}", infoLog);
        }

        glDeleteShader(vs);
        glDeleteShader(fs);

        return program;
    }
}

namespace Earth
{
    Renderer::Renderer()
    {
        std::string vertexSrc = ReadFile("Assets/Shaders/Earth.vert.glsl");
        std::string fragmentSrc = ReadFile("Assets/Shaders/Earth.frag.glsl");

        m_ShaderProgram = CreateShader(vertexSrc, fragmentSrc);
    }

    Renderer::~Renderer()
    {
        if (m_ShaderProgram)
            glDeleteProgram(m_ShaderProgram);
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

    void Renderer::Draw(const glm::mat4& viewProjection)
    {
        if (m_IndexCount == 0)
            return;

        glUseProgram(m_ShaderProgram);

        GLint loc = glGetUniformLocation(m_ShaderProgram, "u_ViewProjection");
        glUniformMatrix4fv(loc, 1, GL_FALSE, &viewProjection[0][0]);

        glBindVertexArray(m_VAO);
        glDrawElements(GL_TRIANGLES, m_IndexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
}
