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

      private:
        void UpdateViewMatrix();

        float m_Width;
        float m_Height;

        // Orbit parameters
        float m_Distance = 3.0f;
        float m_Yaw = 0.0f;
        float m_Pitch = 0.0f;

        glm::vec3 m_Target = {0.0f, 0.0f, 0.0f};
        glm::vec3 m_Position;
        glm::vec3 m_Up = {0.0f, 1.0f, 0.0f};

        // Input state
        bool m_IsDragging = false;
        float m_LastMouseX = 0.0f;
        float m_LastMouseY = 0.0f;
    };
}
