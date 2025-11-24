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
                float cosH = cos(m_Heading);
                float sinH = sin(m_Heading);

                // Forward Vector (in Lon/Lat space): (-sinH, cosH)
                // Right Vector (in Lon/Lat space): (cosH, sinH)

                // Drag Right (deltaX > 0) -> Move Camera Left (-Right Vector)
                // Drag Down (deltaY > 0) -> Move Camera Forward (+Forward Vector)
                float moveX = -deltaX;
                float moveY = deltaY;

                float dLon = moveX * cosH + moveY * (-sinH);
                float dLat = moveX * sinH + moveY * cosH;

                float panSpeed = 0.0025f * m_Range;
                m_TargetLon += dLon * panSpeed;
                m_TargetLat += dLat * panSpeed;

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

    void Camera::SetTargetLonLat(float lon, float lat)
    {
        m_TargetLon = lon;
        m_TargetLat = lat;
        UpdateViewMatrix();
    }

    void Camera::SetPosition(const glm::vec3& position)
    {
        // Move camera to 'position' while keeping the same Target.
        // Update Range, Heading, and Tilt based on the new vector (Position - Target).

        glm::vec3 offset = position - m_TargetPosition;
        m_Range = glm::length(offset);

        if (m_Range < 0.00001f)
        {
            m_Range = 0.00001f;
            UpdateViewMatrix();
            return;
        }

        glm::vec3 dir = glm::normalize(offset);

        // Decompose 'dir' into Heading and Tilt relative to the Target's local frame (East, North, Up).

        // 1. Calculate Local Basis Vectors at Target
        glm::vec3 up = glm::normalize(m_TargetPosition); // Surface Normal

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

        // 2. Project 'dir' onto this basis
        float x = glm::dot(dir, east);
        float y = glm::dot(dir, north);
        float z = glm::dot(dir, up);

        // 3. Convert (x, y, z) to Heading and Tilt
        // z = cos(Tilt)
        m_Tilt = std::acos(std::clamp(z, -1.0f, 1.0f));

        // x = sin(Tilt) * sin(Heading)
        // y = -sin(Tilt) * cos(Heading)
        if (std::abs(std::sin(m_Tilt)) > 0.001f)
        {
            float sinT = std::sin(m_Tilt);
            float sinH = x / sinT;
            float cosH = -y / sinT;
            m_Heading = std::atan2(sinH, cosH);
        }

        UpdateViewMatrix();
    }

    void Camera::SetTargetPosition(const glm::vec3& position)
    {
        m_TargetPosition = position;
        // Convert to Lon/Lat
        m_TargetLon = std::atan2(position.x, position.z);
        m_TargetLat = std::asin(position.y);
        UpdateViewMatrix();
    }

    void Camera::SetPositionLonLatAlt(float lon, float lat, float alt)
    {
        // Interpret as "Move the camera to this location and look down".
        // Set Target = (lon, lat), Range = alt.
        m_TargetLon = lon;
        m_TargetLat = lat;
        m_Range = alt;
        UpdateViewMatrix();
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
        // Construct rotation matrix from basis
        glm::mat4 basis = glm::mat4(1.0f);
        basis[0] = glm::vec4(east, 0.0f);
        basis[1] = glm::vec4(north, 0.0f);
        basis[2] = glm::vec4(up, 0.0f);

        // Local rotation
        // Tilt rotates around X (East)
        // Heading rotates around Z (Up)
        glm::mat4 rotTilt = glm::rotate(glm::mat4(1.0f), m_Tilt, glm::vec3(1, 0, 0));
        glm::mat4 rotHeading = glm::rotate(glm::mat4(1.0f), m_Heading, glm::vec3(0, 0, 1));

        glm::vec3 localCamDir = glm::vec3(rotHeading * rotTilt * glm::vec4(0, 0, 1, 0));

        // Transform to World Space
        glm::vec3 worldCamOffset = glm::vec3(basis * glm::vec4(localCamDir, 0.0f));

        m_Position = m_TargetPosition + worldCamOffset * m_Range;

        // 4. Calculate View Matrix
        glm::vec3 localCamUp = glm::vec3(rotHeading * rotTilt * glm::vec4(0, 1, 0, 0));
        glm::vec3 worldCamUp = glm::vec3(basis * glm::vec4(localCamUp, 0.0f));

        m_ViewMatrix = glm::lookAt(m_Position, m_TargetPosition, worldCamUp);
    }
}
