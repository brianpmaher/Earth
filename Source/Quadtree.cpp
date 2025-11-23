#include "Quadtree.hpp"
#include "Mercator.hpp"

#include <cmath>
#include <glm/gtc/constants.hpp>

namespace Earth
{
    QuadtreeNode::QuadtreeNode(QuadtreeNode* parent, int x, int y, int z, Tileset& tileset)
        : m_Parent(parent), m_X(x), m_Y(y), m_Z(z), m_Tileset(tileset)
    {
        m_Tile = m_Tileset.LoadTile(x, y, z);
    }

    QuadtreeNode::~QuadtreeNode()
    {
    }

    void QuadtreeNode::Update(const Camera& camera)
    {
        if (ShouldSplit(camera))
        {
            if (m_Children.empty())
            {
                Split();
            }

            for (auto& child : m_Children)
            {
                child->Update(camera);
            }
        }
        else
        {
            if (!m_Children.empty())
            {
                Merge();
            }
        }
    }

    void QuadtreeNode::Draw(Renderer& renderer, const glm::mat4& viewProjection)
    {
        if (m_Children.empty())
        {
            if (m_Tile)
            {
                m_Tile->Bind(0);
                if (m_Tile->IsLoaded())
                {
                    renderer.DrawTile(viewProjection, m_X, m_Y, m_Z, true);
                }
            }
        }
        else
        {
            for (auto& child : m_Children)
            {
                child->Draw(renderer, viewProjection);
            }
        }
    }

    void QuadtreeNode::Split()
    {
        int nextZ = m_Z + 1;
        int nextX = m_X * 2;
        int nextY = m_Y * 2;

        m_Children.push_back(std::make_unique<QuadtreeNode>(this, nextX, nextY, nextZ, m_Tileset));
        m_Children.push_back(std::make_unique<QuadtreeNode>(this, nextX + 1, nextY, nextZ, m_Tileset));
        m_Children.push_back(std::make_unique<QuadtreeNode>(this, nextX, nextY + 1, nextZ, m_Tileset));
        m_Children.push_back(std::make_unique<QuadtreeNode>(this, nextX + 1, nextY + 1, nextZ, m_Tileset));
    }

    void QuadtreeNode::Merge()
    {
        m_Children.clear();
    }

    bool QuadtreeNode::ShouldSplit(const Camera& camera) const
    {
        if (m_Z >= 21)
            return false;

        // Calculate center of tile in 3D space
        float scale = 1.0f / (float)(1 << m_Z);
        glm::vec2 centerUV = glm::vec2((float)m_X + 0.5f, (float)m_Y + 0.5f) * scale;
        glm::vec3 centerPos = Mercator::UVToPosition(centerUV, 1.0f);

        glm::mat4 view = camera.GetViewMatrix();
        glm::vec3 camPos = glm::vec3(glm::inverse(view)[3]);

        float dist = glm::distance(centerPos, camPos);

        // Split if distance is small relative to tile size
        // Tile size is roughly proportional to scale (1 / 2^z)
        return dist < 2.5f * scale;
    }

    Quadtree::Quadtree(Tileset& tileset) : m_Tileset(tileset)
    {
        m_Root = std::make_unique<QuadtreeNode>(nullptr, 0, 0, 0, m_Tileset);
    }

    void Quadtree::Update(const Camera& camera)
    {
        m_Root->Update(camera);
    }

    void Quadtree::Draw(Renderer& renderer, const glm::mat4& viewProjection)
    {
        m_Root->Draw(renderer, viewProjection);
    }
}
