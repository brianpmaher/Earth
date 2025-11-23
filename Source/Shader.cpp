#include "Shader.hpp"

#include <fstream>
#include <print>
#include <sstream>

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
    Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath)
    {
        std::string vertexSrc = ReadFile(vertexPath);
        std::string fragmentSrc = ReadFile(fragmentPath);

        m_RendererID = CreateShader(vertexSrc, fragmentSrc);
    }

    Shader::~Shader()
    {
        glDeleteProgram(m_RendererID);
    }

    void Shader::Bind() const
    {
        glUseProgram(m_RendererID);
    }

    void Shader::Unbind() const
    {
        glUseProgram(0);
    }

    void Shader::SetBool(const std::string& name, bool value) const
    {
        glUniform1i(glGetUniformLocation(m_RendererID, name.c_str()), (int)value);
    }

    void Shader::SetInt(const std::string& name, int value) const
    {
        glUniform1i(glGetUniformLocation(m_RendererID, name.c_str()), value);
    }
}
