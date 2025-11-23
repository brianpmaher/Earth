#include "Tileset.hpp"
#include "HTTP.hpp"
#include "Image.hpp"
#include "Logger.hpp"

#include <format>
#include <print>

namespace Earth
{
    static Logger s_Logger("Tileset");

    Tile::Tile(int x, int y, int z, const URL& urlTemplate) : X(x), Y(y), Z(z)
    {
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

        try
        {
            std::string imageData = HTTP::Fetch(url);
            Image image(imageData);

            if (image.GetData())
            {
                glGenTextures(1, &TextureID);
                glBindTexture(GL_TEXTURE_2D, TextureID);

                GLenum format = GL_RGB;
                if (image.GetChannels() == 4)
                    format = GL_RGBA;

                glTexImage2D(GL_TEXTURE_2D, 0, format, image.GetWidth(), image.GetHeight(), 0, format, GL_UNSIGNED_BYTE,
                             image.GetData());
                glGenerateMipmap(GL_TEXTURE_2D);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                s_Logger.Info("Loaded tile texture: {} ({}x{})", TextureID, image.GetWidth(), image.GetHeight());
            }
        }
        catch (const std::exception& e)
        {
            s_Logger.Error("Failed to fetch tile: {}", e.what());
        }
    }

    Tile::~Tile()
    {
        if (TextureID)
        {
            glDeleteTextures(1, &TextureID);
        }
    }

    void Tile::Bind(int slot) const
    {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, TextureID);
    }

    Tileset::Tileset(const URL& urlTemplate) : m_UrlTemplate(urlTemplate)
    {
    }

    std::shared_ptr<Tile> Tileset::LoadTile(int x, int y, int z)
    {
        return std::make_shared<Tile>(x, y, z, m_UrlTemplate);
    }
}
