#pragma once

#include <string>

namespace Earth
{
    class Image
    {
      public:
        Image() = default;
        Image(const std::string& data, bool flipVertically = false);
        Image(Image&& other) noexcept;
        Image& operator=(Image&& other) noexcept;

        ~Image();

        int GetWidth() const
        {
            return m_Width;
        }
        int GetHeight() const
        {
            return m_Height;
        }
        int GetChannels() const
        {
            return m_Channels;
        }
        const unsigned char* GetData() const
        {
            return m_Data;
        }

      private:
        int m_Width = 0;
        int m_Height = 0;
        int m_Channels = 0;
        unsigned char* m_Data = nullptr;
    };
}
