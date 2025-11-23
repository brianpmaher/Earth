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
        if (m_Tile)
        {
            m_Tile->CheckLoad();
        }

        if (ShouldSplit(camera))
        {
            if (m_Children.empty())
            {
                Split();
            }

            bool childrenReady = true;
            for (auto& child : m_Children)
            {
                child->Update(camera);
                if (!child->IsRenderable())
                {
                    childrenReady = false;
                }
            }
            m_AllChildrenRenderable = childrenReady;
        }
        else
        {
            if (!m_Children.empty())
            {
                Merge();
            }
            m_AllChildrenRenderable = false;
        }

        m_IsRenderable = m_AllChildrenRenderable || (m_Tile && m_Tile->IsLoaded());
    }

    void QuadtreeNode::Draw(Renderer& renderer, const glm::mat4& viewProjection)
    {
        if (m_AllChildrenRenderable)
        {
            for (auto& child : m_Children)
            {
                child->Draw(renderer, viewProjection);
            }
        }
        else
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

        float scale = 1.0f / (float)(1 << m_Z);

        glm::mat4 view = camera.GetViewMatrix();
        glm::vec3 camPos = glm::vec3(glm::inverse(view)[3]);

        // Check distance to center and corners to handle seams correctly
        glm::vec2 centerUV = glm::vec2((float)m_X + 0.5f, (float)m_Y + 0.5f) * scale;
        float minDist = glm::distance(Mercator::UVToPosition(centerUV, 1.0f), camPos);

        glm::vec2 corners[] = {
            glm::vec2((float)m_X, (float)m_Y) * scale, glm::vec2((float)m_X + 1.0f, (float)m_Y) * scale,
            glm::vec2((float)m_X, (float)m_Y + 1.0f) * scale, glm::vec2((float)m_X + 1.0f, (float)m_Y + 1.0f) * scale};

        for (const auto& uv : corners)
        {
            float dist = glm::distance(Mercator::UVToPosition(uv, 1.0f), camPos);
            if (dist < minDist)
                minDist = dist;
        }

        // Split if distance is small relative to tile size
        // Tile size is roughly proportional to scale (1 / 2^z)
        return minDist < 2.5f * scale;
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
