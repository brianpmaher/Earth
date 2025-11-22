#pragma once

#include "Mesh.hpp"

#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>

namespace Earth
{
    class Renderer
    {
      public:
        Renderer();
        ~Renderer();

        void UploadMesh(const Mesh& mesh);

        void Draw(const glm::mat4& viewProjection);

      private:
        GLuint m_ShaderProgram = 0;
        GLuint m_VAO = 0;
        GLuint m_VBO = 0;
        GLuint m_EBO = 0;
        GLsizei m_IndexCount = 0;
    };
}
