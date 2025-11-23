#pragma once

#include "URL.hpp"

#include <OpenGL/gl3.h>
#include <memory>
#include <string>

namespace Earth
{
    struct Tile
    {
        Tile(int x, int y, int z, const URL& urlTemplate);
        ~Tile();

        void Bind(int slot = 0) const;

        int X, Y, Z;
        GLuint TextureID = 0;
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
