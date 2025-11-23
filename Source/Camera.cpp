#include "Camera.hpp"

#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

namespace Earth
{
    Camera::Camera(float width, float height) : m_Width(width), m_Height(height)
    {
        UpdateViewMatrix();
    }

    void Camera::Update(float deltaTime)
    {
        // Smooth movement could go here
    }

    void Camera::HandleEvent(const SDL_Event& event)
    {
        if (event.type == SDL_EVENT_MOUSE_WHEEL)
        {
            m_Distance -= event.wheel.y * 0.2f;
            m_Distance = std::max(1.1f, std::min(m_Distance, 10.0f));
            UpdateViewMatrix();
        }
        else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
        {
            if (event.button.button == SDL_BUTTON_LEFT)
            {
                m_IsDragging = true;
                m_LastMouseX = event.button.x;
                m_LastMouseY = event.button.y;
            }
        }
        else if (event.type == SDL_EVENT_MOUSE_BUTTON_UP)
        {
            if (event.button.button == SDL_BUTTON_LEFT)
            {
                m_IsDragging = false;
            }
        }
        else if (event.type == SDL_EVENT_MOUSE_MOTION)
        {
            if (m_IsDragging)
            {
                float deltaX = event.motion.x - m_LastMouseX;
                float deltaY = event.motion.y - m_LastMouseY;

                m_LastMouseX = event.motion.x;
                m_LastMouseY = event.motion.y;

                float rotationSpeed = 0.0025f * (m_Distance - 1.0f);
                m_Yaw -= deltaX * rotationSpeed;
                m_Pitch += deltaY * rotationSpeed;

                // Clamp pitch to avoid flipping
                m_Pitch = std::clamp(m_Pitch, -1.5f, 1.5f);

                UpdateViewMatrix();
            }
        }
    }

    void Camera::Resize(float width, float height)
    {
        m_Width = width;
        m_Height = height;
    }

    glm::mat4 Camera::GetViewMatrix() const
    {
        return glm::lookAt(m_Position, m_Target, m_Up);
    }

    glm::mat4 Camera::GetProjectionMatrix() const
    {
        return glm::perspective(glm::radians(45.0f), m_Width / m_Height, 0.1f, 100.0f);
    }

    void Camera::UpdateViewMatrix()
    {
        m_Position.x = m_Target.x + m_Distance * cos(m_Pitch) * sin(m_Yaw);
        m_Position.y = m_Target.y + m_Distance * sin(m_Pitch);
        m_Position.z = m_Target.z + m_Distance * cos(m_Pitch) * cos(m_Yaw);
    }
}
