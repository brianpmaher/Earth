#pragma once

#include "Image.hpp"
#include "URL.hpp"

#include <OpenGL/gl3.h>
#include <future>
#include <memory>
#include <string>

namespace Earth
{
    struct Tile
    {
        Tile(int x, int y, int z, const URL& urlTemplate);
        ~Tile();

        void Bind(int slot = 0);
        bool IsLoaded() const
        {
            return TextureID != 0;
        }

        int X, Y, Z;
        GLuint TextureID = 0;

      private:
        void CheckLoad();

        std::future<Image> m_Future;
        bool m_IsLoading = true;
    };

    class Tileset
    {
      public:
        Tileset(const URL& urlTemplate);

        std::shared_ptr<Tile> LoadTile(int x, int y, int z);

      private:
        URL m_UrlTemplate;
    };
}
