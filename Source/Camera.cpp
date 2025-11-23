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
            float zoomSpeed = std::max(0.00001f, m_Range * 0.1f);
            m_Range -= event.wheel.y * zoomSpeed;
            m_Range = std::max(0.00001f, std::min(m_Range, 9.0f));
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
            else if (event.button.button == SDL_BUTTON_RIGHT)
            {
                m_IsPivoting = true;
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
            else if (event.button.button == SDL_BUTTON_RIGHT)
            {
                m_IsPivoting = false;
            }
        }
        else if (event.type == SDL_EVENT_MOUSE_MOTION)
        {
            float deltaX = event.motion.x - m_LastMouseX;
            float deltaY = event.motion.y - m_LastMouseY;

            m_LastMouseX = event.motion.x;
            m_LastMouseY = event.motion.y;

            if (m_IsDragging)
            {
                // Pan relative to camera view
                // Rotate delta by Heading
                float cosH = cos(m_Heading);
                float sinH = sin(m_Heading);

                // Screen X (Right) corresponds to East (if Heading 0)
                // Screen Y (Down) corresponds to South (if Heading 0) -> Lat decreases
                // Wait, dragging Down usually moves map Down -> Camera moves North -> Lat increases?
                // Let's stick to: Dragging moves the CAMERA.
                // Drag Left (dX < 0) -> Camera moves Left (West if H=0) -> Lon decreases.
                // Drag Up (dY < 0) -> Camera moves Up (North if H=0) -> Lat increases.

                float dX_World = deltaX * cosH - deltaY * sinH;
                float dY_World = deltaX * sinH + deltaY * cosH;

                float panSpeed = 0.0025f * m_Range;
                m_TargetLon -= dX_World * panSpeed;
                m_TargetLat += dY_World * panSpeed;

                // Clamp Lat
                m_TargetLat = std::clamp(m_TargetLat, -1.5f, 1.5f);

                UpdateViewMatrix();
            }
            else if (m_IsPivoting)
            {
                m_Heading -= deltaX * 0.005f;
                m_Tilt -= deltaY * 0.005f;

                // Clamp Tilt (0 to 90 degrees approx)
                // 0 is looking down. PI/2 is looking at horizon.
                m_Tilt = std::clamp(m_Tilt, 0.0f, 1.5f);

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
        return m_ViewMatrix;
    }

    glm::mat4 Camera::GetProjectionMatrix() const
    {
        float nearPlane = std::max(0.000001f, m_Range * 0.1f);
        return glm::perspective(glm::radians(45.0f), m_Width / m_Height, nearPlane, 100.0f);
    }

    void Camera::UpdateViewMatrix()
    {
        // 1. Calculate Target Position on Sphere (Radius 1.0)
        m_TargetPosition.x = cos(m_TargetLat) * sin(m_TargetLon);
        m_TargetPosition.y = sin(m_TargetLat);
        m_TargetPosition.z = cos(m_TargetLat) * cos(m_TargetLon);

        // 2. Calculate Local Basis Vectors at Target
        glm::vec3 up = glm::normalize(m_TargetPosition); // Surface Normal

        // Handle pole singularity
        glm::vec3 north;
        if (std::abs(up.y) > 0.99f)
        {
            north = glm::vec3(1.0f, 0.0f, 0.0f); // Arbitrary at pole
        }
        else
        {
            north = glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f) - up * up.y);
        }

        glm::vec3 east = glm::cross(north, up);

        // 3. Calculate Camera Position relative to Target
        // We want to rotate "Up" vector by Tilt (around East) and Heading (around Up)
        // Start with vector pointing UP (from Target to Camera if Tilt=0)
        // Rotate by Tilt around East axis (Positive Tilt = Camera moves South/Back)
        // Rotate by Heading around Up axis

        // Construct rotation matrix from basis
        glm::mat4 basis = glm::mat4(1.0f);
        basis[0] = glm::vec4(east, 0.0f);
        basis[1] = glm::vec4(north, 0.0f);
        basis[2] = glm::vec4(up, 0.0f);

        // Local rotation
        // Tilt rotates around X (East)
        // Heading rotates around Z (Up)
        // Note: In our basis, Z is Up. X is East. Y is North.
        // Wait, let's check basis construction.
        // basis[0] = X = East
        // basis[1] = Y = North
        // basis[2] = Z = Up

        // We want camera vector.
        // Start with (0,0,1) (Up)
        // Tilt: Rotate around X (East). Positive Tilt moves vector towards -Y (South).
        // Heading: Rotate around Z (Up).

        glm::mat4 rotTilt = glm::rotate(glm::mat4(1.0f), m_Tilt, glm::vec3(1, 0, 0));
        glm::mat4 rotHeading = glm::rotate(glm::mat4(1.0f), m_Heading, glm::vec3(0, 0, 1));

        glm::vec3 localCamDir = glm::vec3(rotHeading * rotTilt * glm::vec4(0, 0, 1, 0));

        // Transform to World Space
        glm::vec3 worldCamOffset = glm::vec3(basis * glm::vec4(localCamDir, 0.0f));

        m_Position = m_TargetPosition + worldCamOffset * m_Range;

        // 4. Calculate View Matrix
        // Up vector for LookAt should be the "Up" of the camera frame
        // Local Up for camera is (0,1,0) (North) rotated?
        // No, Camera Up is usually "Up" (Z) rotated by Tilt-90?
        // Let's just use the transformed Y axis of the camera frame.
        // Local Camera Up = (0, 1, 0) (North) rotated by Heading/Tilt?
        // Actually, if we look down (Tilt 0), Camera Up is North (0,1,0).
        // If we tilt back, Camera Up tilts back too.

        glm::vec3 localCamUp = glm::vec3(rotHeading * rotTilt * glm::vec4(0, 1, 0, 0));
        glm::vec3 worldCamUp = glm::vec3(basis * glm::vec4(localCamUp, 0.0f));

        m_ViewMatrix = glm::lookAt(m_Position, m_TargetPosition, worldCamUp);
    }
}
