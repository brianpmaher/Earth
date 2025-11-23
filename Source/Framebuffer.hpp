#pragma once

#include <OpenGL/gl3.h>

namespace Earth
{
    class Framebuffer
    {
      public:
        Framebuffer(int width, int height);
        ~Framebuffer();

        void Bind();
        void Unbind();
        void Resize(int width, int height);

        GLuint GetTextureID() const
        {
            return m_TextureID;
        }
        int GetWidth() const
        {
            return m_Width;
        }
        int GetHeight() const
        {
            return m_Height;
        }

      private:
        GLuint m_FBO = 0;
        GLuint m_TextureID = 0;
        GLuint m_RBO = 0;
        int m_Width = 0;
        int m_Height = 0;

        void Create();
        void Destroy();
    };
}
