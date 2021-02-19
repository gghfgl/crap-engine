#pragma once

enum CameraDirection
{
    CAMERA_FORWARD,
    CAMERA_BACKWARD,
    CAMERA_LEFT,
    CAMERA_RIGHT,
    CAMERA_UP,
    CAMERA_DOWN
};

struct Camera {
    Camera(glm::vec3 position, glm::vec3 lookAt, glm::vec3 upVector, float32 fov, float32 aspect, float32 nearPlane, float32 farPlane);
    void SetCameraView(glm::vec3 position, glm::vec3 lookAt, glm::vec3 upVector);
    void UpdatePositionFromDirection(CameraDirection direction, float32 deltaTime, float32 acceleration);
    void UpdatePositionFromAngle(float32 xoffset, float32 yoffset, bool constrainPitch);
    void UpdateProjectionFromFOV(float32 yoffset);

    glm::vec3 m_position;
    glm::vec3 m_lookAt;
    glm::vec3 m_worldUp;
    
    glm::vec3 m_upVector;
    glm::vec3 m_rightVector;
    glm::vec3 m_frontVector;

    glm::mat4 m_projectionMatrix;
    glm::mat4 m_viewMatrix;

    float32 m_yaw = -90.0f;
    float32 m_pitch = -60.0f;
    float32 m_speed = 10.0f;
    float32 m_sensitivity = 0.1f;

    float32 m_fov = 45.0f;
    float32 m_aspect;
    float32 m_nearPlane = 1.0f;
    float32 m_farPlane = 100.0f;

private:
    void update_view_matrix();
    void update_projection_matrix();
    void update_normalize_vectors();
};
