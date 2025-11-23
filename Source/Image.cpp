#include "Image.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <webp/decode.h>

#include <stdexcept>
#include <vector>

namespace Earth
{
    Image::Image(const std::string& data, bool flipVertically)
    {
        // Check if it is WebP
        if (WebPGetInfo(reinterpret_cast<const uint8_t*>(data.data()), data.size(), &m_Width, &m_Height))
        {
            m_Channels = 4;
            m_Data = WebPDecodeRGBA(reinterpret_cast<const uint8_t*>(data.data()), data.size(), &m_Width, &m_Height);
            m_IsWebP = true;

            if (!m_Data)
            {
                throw std::runtime_error("Failed to decode WebP image");
            }

            if (flipVertically)
            {
                int stride = m_Width * m_Channels;
                std::vector<unsigned char> row(stride);
                for (int y = 0; y < m_Height / 2; ++y)
                {
                    unsigned char* top = m_Data + y * stride;
                    unsigned char* bottom = m_Data + (m_Height - 1 - y) * stride;
                    memcpy(row.data(), top, stride);
                    memcpy(top, bottom, stride);
                    memcpy(bottom, row.data(), stride);
                }
            }
        }
        else
        {
            stbi_set_flip_vertically_on_load(flipVertically);

            m_Data = stbi_load_from_memory(reinterpret_cast<const unsigned char*>(data.data()),
                                           static_cast<int>(data.size()), &m_Width, &m_Height, &m_Channels, 0);

            if (!m_Data)
            {
                throw std::runtime_error(std::string("Failed to load image from memory: ") + stbi_failure_reason());
            }
        }
    }

    Image::Image(Image&& other) noexcept
        : m_Width(other.m_Width), m_Height(other.m_Height), m_Channels(other.m_Channels), m_Data(other.m_Data),
          m_IsWebP(other.m_IsWebP)
    {
        other.m_Data = nullptr;
        other.m_Width = 0;
        other.m_Height = 0;
        other.m_Channels = 0;
        other.m_IsWebP = false;
    }

    Image& Image::operator=(Image&& other) noexcept
    {
        if (this != &other)
        {
            if (m_Data)
            {
                if (m_IsWebP)
                    WebPFree(m_Data);
                else
                    stbi_image_free(m_Data);
            }

            m_Width = other.m_Width;
            m_Height = other.m_Height;
            m_Channels = other.m_Channels;
            m_Data = other.m_Data;
            m_IsWebP = other.m_IsWebP;

            other.m_Data = nullptr;
            other.m_Width = 0;
            other.m_Height = 0;
            other.m_Channels = 0;
            other.m_IsWebP = false;
        }
        return *this;
    }

    Image::~Image()
    {
        if (m_Data)
        {
            if (m_IsWebP)
                WebPFree(m_Data);
            else
                stbi_image_free(m_Data);
        }
    }
}
