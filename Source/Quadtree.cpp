#include "Quadtree.hpp"
#include "Mercator.hpp"

#include <cmath>
#include <glm/gtc/constants.hpp>

namespace Earth
{
    QuadtreeNode::QuadtreeNode(QuadtreeNode* parent, int x, int y, int z, Tileset& satelliteTileset,
                               Tileset& terrainTileset)
        : m_Parent(parent), m_X(x), m_Y(y), m_Z(z), m_SatelliteTileset(satelliteTileset),
          m_TerrainTileset(terrainTileset)
    {
    }

    QuadtreeNode::~QuadtreeNode()
    {
    }

    void QuadtreeNode::Update(const Camera& camera)
    {
        m_IsVisible = CheckVisibility(camera);

        if (!m_IsVisible)
        {
            if (!m_Children.empty())
                Merge();
            m_IsRenderable = false;
            return;
        }

        if (!m_SatelliteTile)
            m_SatelliteTile = m_SatelliteTileset.LoadTile(m_X, m_Y, m_Z);
        if (!m_TerrainTile)
            m_TerrainTile = m_TerrainTileset.LoadTile(m_X, m_Y, m_Z);

        if (m_SatelliteTile)
        {
            m_SatelliteTile->CheckLoad();
        }
        if (m_TerrainTile)
        {
            m_TerrainTile->CheckLoad();
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
                // If child is visible, it must be renderable to be considered ready.
                // If child is NOT visible, it is considered ready (since it won't be drawn).
                if (child->IsVisible() && !child->IsRenderable())
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

        m_IsRenderable = m_AllChildrenRenderable ||
                         (m_SatelliteTile && m_SatelliteTile->IsLoaded() && m_TerrainTile && m_TerrainTile->IsLoaded());
    }

    void QuadtreeNode::Draw(Renderer& renderer, const glm::mat4& viewProjection)
    {
        if (!m_IsVisible)
            return;

        if (m_AllChildrenRenderable)
        {
            for (auto& child : m_Children)
            {
                child->Draw(renderer, viewProjection);
            }
        }
        else
        {
            if (m_SatelliteTile && m_TerrainTile)
            {
                m_SatelliteTile->Bind(0);
                m_TerrainTile->Bind(1);
                if (m_SatelliteTile->IsLoaded() && m_TerrainTile->IsLoaded())
                {
                    bool showGrid = false;
                    renderer.DrawTile(viewProjection, m_X, m_Y, m_Z, showGrid);
                }
            }
        }
    }

    void QuadtreeNode::Split()
    {
        int nextZ = m_Z + 1;
        int nextX = m_X * 2;
        int nextY = m_Y * 2;

        m_Children.push_back(
            std::make_unique<QuadtreeNode>(this, nextX, nextY, nextZ, m_SatelliteTileset, m_TerrainTileset));
        m_Children.push_back(
            std::make_unique<QuadtreeNode>(this, nextX + 1, nextY, nextZ, m_SatelliteTileset, m_TerrainTileset));
        m_Children.push_back(
            std::make_unique<QuadtreeNode>(this, nextX, nextY + 1, nextZ, m_SatelliteTileset, m_TerrainTileset));
        m_Children.push_back(
            std::make_unique<QuadtreeNode>(this, nextX + 1, nextY + 1, nextZ, m_SatelliteTileset, m_TerrainTileset));
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
        glm::vec3 camPos = camera.GetPosition();

        float minDist = std::numeric_limits<float>::max();

        for (int y = 0; y <= 2; ++y)
        {
            for (int x = 0; x <= 2; ++x)
            {
                glm::vec2 uv = glm::vec2((float)m_X + x * 0.5f, (float)m_Y + y * 0.5f) * scale;
                glm::vec3 p = Mercator::UVToPosition(uv, 1.0f);
                float d = glm::distance(p, camPos);
                if (d < minDist)
                {
                    minDist = d;
                }
            }
        }

        // Avoid division by zero
        minDist = std::max(minDist, 0.00001f);

        float tileWidth = glm::pi<float>() * 2.0f * scale;
        float sse = (tileWidth * camera.GetHeight()) / (2.0f * minDist * std::tan(camera.GetFOV() / 2.0f));

        bool isSplit = !m_Children.empty();
        float threshold = isSplit ? 200.0f : 250.0f;

        return sse > threshold;
    }
    bool QuadtreeNode::CheckVisibility(const Camera& camera) const
    {
        if (m_Z < 1)
            return true;

        float scale = 1.0f / (float)(1 << m_Z);
        glm::vec3 min(std::numeric_limits<float>::max());
        glm::vec3 max(std::numeric_limits<float>::lowest());

        glm::vec3 camPos = camera.GetPosition();
        bool anyVisible = false;

        for (int y = 0; y <= 4; ++y)
        {
            for (int x = 0; x <= 4; ++x)
            {
                glm::vec2 uv = glm::vec2((float)m_X + (float)x / 4.0f, (float)m_Y + (float)y / 4.0f) * scale;
                glm::vec3 p = Mercator::UVToPosition(uv);

                if (glm::dot(glm::normalize(p), camPos) > 1.0f - 0.05f)
                {
                    anyVisible = true;
                }

                min = glm::min(min, p);
                max = glm::max(max, p);
            }
        }

        if (!anyVisible)
            return false;

        min -= glm::vec3(0.002f);
        max += glm::vec3(0.002f);

        return camera.GetFrustum().IsBoxVisible(min, max);
    }

    Quadtree::Quadtree(Tileset& satelliteTileset, Tileset& terrainTileset)
        : m_SatelliteTileset(satelliteTileset), m_TerrainTileset(terrainTileset)
    {
        m_Root = std::make_unique<QuadtreeNode>(nullptr, 0, 0, 0, m_SatelliteTileset, m_TerrainTileset);
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
