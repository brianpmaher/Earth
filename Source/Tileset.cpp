#include "Tileset.hpp"
#include "HTTP.hpp"
#include "Image.hpp"
#include "Logger.hpp"

#include <format>
#include <print>

namespace Earth
{
    static Logger s_Logger("Tileset");

    std::atomic<int> Tile::s_TotalTiles = 0;
    std::atomic<int> Tile::s_LoadingTiles = 0;
    std::atomic<int> Tile::s_LoadedTiles = 0;

    Tile::Tile(int x, int y, int z, const URL& urlTemplate, bool generateMipmaps)
        : X(x), Y(y), Z(z), m_GenerateMipmaps(generateMipmaps)
    {
        s_TotalTiles++;
        s_LoadingTiles++;

        std::string url = urlTemplate.Get();
        // Simple replacement for now. In a real app, use a proper template engine or regex.
        // The template is like "https://.../{z}/{x}/{y}.jpg"

        auto replace = [&](const std::string& key, int value) {
            std::string keyStr = "{" + key + "}";
            size_t pos = url.find(keyStr);
            if (pos != std::string::npos)
            {
                url.replace(pos, keyStr.length(), std::to_string(value));
            }
        };

        replace("z", z);
        replace("x", x);
        replace("y", y);

        s_Logger.Info("Fetching tile: {}", url);

        m_Future = std::async(std::launch::async, [url]() {
            try
            {
                std::string imageData = HTTP::Fetch(url);
                return Image(imageData);
            }
            catch (const std::exception& e)
            {
                s_Logger.Error("Failed to fetch tile: {}", e.what());
                return Image();
            }
        });
    }

    Tile::~Tile()
    {
        s_TotalTiles--;
        if (m_IsLoading)
            s_LoadingTiles--;
        if (TextureID)
            s_LoadedTiles--;

        if (TextureID)
        {
            glDeleteTextures(1, &TextureID);
        }
    }

    void Tile::Bind(int slot)
    {
        CheckLoad();

        if (TextureID)
        {
            glActiveTexture(GL_TEXTURE0 + slot);
            glBindTexture(GL_TEXTURE_2D, TextureID);
        }
    }

    void Tile::CheckLoad()
    {
        if (m_IsLoading && m_Future.valid())
        {
            if (m_Future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
            {
                Image image = m_Future.get();
                m_IsLoading = false;
                s_LoadingTiles--;

                if (image.GetData())
                {
                    s_LoadedTiles++;
                    glGenTextures(1, &TextureID);
                    glBindTexture(GL_TEXTURE_2D, TextureID);

                    GLenum format = GL_RGB;
                    if (image.GetChannels() == 4)
                        format = GL_RGBA;

                    glTexImage2D(GL_TEXTURE_2D, 0, format, image.GetWidth(), image.GetHeight(), 0, format,
                                 GL_UNSIGNED_BYTE, image.GetData());

                    if (m_GenerateMipmaps)
                    {
                        glGenerateMipmap(GL_TEXTURE_2D);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    }
                    else
                    {
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                    }

                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

                    s_Logger.Info("Loaded tile texture: {} ({}x{})", TextureID, image.GetWidth(), image.GetHeight());
                }
            }
        }
    }

    Tileset::Tileset(const URL& urlTemplate, bool generateMipmaps)
        : m_UrlTemplate(urlTemplate), m_GenerateMipmaps(generateMipmaps)
    {
    }

    std::shared_ptr<Tile> Tileset::LoadTile(int x, int y, int z)
    {
        return std::make_shared<Tile>(x, y, z, m_UrlTemplate, m_GenerateMipmaps);
    }
}
