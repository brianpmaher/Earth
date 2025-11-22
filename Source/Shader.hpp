#pragma once

#include <OpenGL/gl3.h>

#include <string>

namespace Earth
{
    class Shader
    {
      public:
        Shader(const std::string& vertexPath, const std::string& fragmentPath);
        ~Shader();

        void Bind() const;

        void Unbind() const;

        GLuint GetRendererID() const
        {
            return m_RendererID;
        }

      private:
        GLuint m_RendererID;
    };
}
