#pragma once

#include <SDL3/SDL_events.h>
#include <glm/glm.hpp>

namespace Earth
{
    class Camera
    {
      public:
        Camera(float width, float height);

        void Update(float deltaTime);
        void HandleEvent(const SDL_Event& event);
        void Resize(float width, float height);

        glm::mat4 GetViewMatrix() const;
        glm::mat4 GetProjectionMatrix() const;

        glm::vec3 GetPosition() const
        {
            return m_Position;
        }
        glm::vec3 GetTargetPosition() const
        {
            return m_TargetPosition;
        }
        void GetTargetLonLat(float& lon, float& lat) const
        {
            lon = m_TargetLon;
            lat = m_TargetLat;
        }
        void SetTargetLonLat(float lon, float lat);
        float GetRange() const
        {
            return m_Range;
        }
        float GetTargetLon() const
        {
            return m_TargetLon;
        }
        float GetTargetLat() const
        {
            return m_TargetLat;
        }
        float GetHeading() const
        {
            return m_Heading;
        }
        float GetTilt() const
        {
            return m_Tilt;
        }

      private:
        void UpdateViewMatrix();

        float m_Width;
        float m_Height;

        // Orbit parameters
        float m_Range = 2.0f; // Distance from Target
        float m_TargetLon = 0.0f;
        float m_TargetLat = 0.0f;
        float m_Heading = 0.0f;
        float m_Tilt = 0.0f;

        glm::vec3 m_TargetPosition;
        glm::vec3 m_Position;
        glm::vec3 m_Up = {0.0f, 1.0f, 0.0f};
        glm::mat4 m_ViewMatrix;

        // Input state
        bool m_IsDragging = false;
        bool m_IsPivoting = false;
        float m_LastMouseX = 0.0f;
        float m_LastMouseY = 0.0f;
    };
}
