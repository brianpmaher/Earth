#pragma once

#include "Image.hpp"
#include "ThreadPool.hpp"
#include "URL.hpp"

#include <OpenGL/gl3.h>
#include <atomic>
#include <future>
#include <memory>
#include <string>

namespace Earth
{
    struct Tile
    {
        Tile(int x, int y, int z, const URL& urlTemplate, bool generateMipmaps, ThreadPool& threadPool);
        ~Tile();

        void Bind(int slot = 0);
        void CheckLoad();
        bool IsLoaded() const
        {
            return TextureID != 0;
        }

        int X, Y, Z;
        GLuint TextureID = 0;

        static std::atomic<int> s_TotalTiles;
        static std::atomic<int> s_LoadingTiles;
        static std::atomic<int> s_LoadedTiles;

      private:
        std::future<Image> m_Future;
        bool m_IsLoading = true;
        bool m_GenerateMipmaps = false;
    };

    class Tileset
    {
      public:
        Tileset(const URL& urlTemplate, ThreadPool& threadPool, bool generateMipmaps = false);

        std::shared_ptr<Tile> LoadTile(int x, int y, int z);

      private:
        URL m_UrlTemplate;
        bool m_GenerateMipmaps;
        ThreadPool& m_ThreadPool;
    };
}
