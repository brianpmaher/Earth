#include "Image.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <stdexcept>

namespace Earth
{
    Image::Image(const std::string& data, bool flipVertically)
    {
        stbi_set_flip_vertically_on_load(flipVertically);

        m_Data = stbi_load_from_memory(reinterpret_cast<const unsigned char*>(data.data()),
                                       static_cast<int>(data.size()), &m_Width, &m_Height, &m_Channels, 0);

        if (!m_Data)
        {
            throw std::runtime_error("Failed to load image from memory");
        }
    }

    Image::Image(Image&& other) noexcept
        : m_Width(other.m_Width), m_Height(other.m_Height), m_Channels(other.m_Channels), m_Data(other.m_Data)
    {
        other.m_Data = nullptr;
        other.m_Width = 0;
        other.m_Height = 0;
        other.m_Channels = 0;
    }

    Image& Image::operator=(Image&& other) noexcept
    {
        if (this != &other)
        {
            if (m_Data)
            {
                stbi_image_free(m_Data);
            }

            m_Width = other.m_Width;
            m_Height = other.m_Height;
            m_Channels = other.m_Channels;
            m_Data = other.m_Data;

            other.m_Data = nullptr;
            other.m_Width = 0;
            other.m_Height = 0;
            other.m_Channels = 0;
        }
        return *this;
    }

    Image::~Image()
    {
        if (m_Data)
        {
            stbi_image_free(m_Data);
        }
    }
}
