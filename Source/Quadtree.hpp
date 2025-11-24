#pragma once

#include "Camera.hpp"
#include "Renderer.hpp"
#include "Tileset.hpp"

#include <memory>
#include <vector>

namespace Earth
{
    class QuadtreeNode
    {
      public:
        QuadtreeNode(QuadtreeNode* parent, int x, int y, int z, Tileset& satelliteTileset, Tileset& terrainTileset);
        ~QuadtreeNode();

        void Update(const Camera& camera);
        void Draw(Renderer& renderer, const glm::mat4& viewProjection);

        bool IsRenderable() const
        {
            return m_IsRenderable;
        }

        bool IsVisible() const
        {
            return m_IsVisible;
        }

      private:
        void Split();
        void Merge();
        bool ShouldSplit(const Camera& camera) const;
        bool CheckVisibility(const Camera& camera) const;

        QuadtreeNode* m_Parent;
        int m_X, m_Y, m_Z;
        Tileset& m_SatelliteTileset;
        Tileset& m_TerrainTileset;

        std::shared_ptr<Tile> m_SatelliteTile;
        std::shared_ptr<Tile> m_TerrainTile;
        std::vector<std::unique_ptr<QuadtreeNode>> m_Children;

        bool m_IsRenderable = false;
        bool m_IsVisible = true;
        bool m_AllChildrenRenderable = false;
    };

    class Quadtree
    {
      public:
        Quadtree(Tileset& satelliteTileset, Tileset& terrainTileset);

        void Update(const Camera& camera);
        void Draw(Renderer& renderer, const glm::mat4& viewProjection);

      private:
        Tileset& m_SatelliteTileset;
        Tileset& m_TerrainTileset;
        std::unique_ptr<QuadtreeNode> m_Root;
    };
}
