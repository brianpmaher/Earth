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

    Image::~Image()
    {
        if (m_Data)
        {
            stbi_image_free(m_Data);
        }
    }
}
