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
        QuadtreeNode(QuadtreeNode* parent, int x, int y, int z, Tileset& tileset);
        ~QuadtreeNode();

        void Update(const Camera& camera);
        void Draw(Renderer& renderer, const glm::mat4& viewProjection);

        bool IsRenderable() const
        {
            return m_IsRenderable;
        }

      private:
        void Split();
        void Merge();
        bool ShouldSplit(const Camera& camera) const;

        QuadtreeNode* m_Parent;
        int m_X, m_Y, m_Z;
        Tileset& m_Tileset;

        std::shared_ptr<Tile> m_Tile;
        std::vector<std::unique_ptr<QuadtreeNode>> m_Children;

        bool m_IsRenderable = false;
        bool m_AllChildrenRenderable = false;
    };

    class Quadtree
    {
      public:
        Quadtree(Tileset& tileset);

        void Update(const Camera& camera);
        void Draw(Renderer& renderer, const glm::mat4& viewProjection);

      private:
        Tileset& m_Tileset;
        std::unique_ptr<QuadtreeNode> m_Root;
    };
}
