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
    Camera(glm::vec3 position, glm::vec3 lookAt, glm::vec3 upVector, float32 fov, float32 pitch, float32 aspect, float32 nearPlane, float32 farPlane);
    void SetCameraView(glm::vec3 position, glm::vec3 lookAt, glm::vec3 upVector);
    void UpdatePositionFromDirection(CameraDirection direction, float32 deltaTime, float32 acceleration);
    void UpdatePositionFromAngle(float32 xoffset, float32 yoffset, bool constrainPitch);
    void UpdateArcballFromAngle(float32 xoffset, float32 yoffset, bool constrainPitch);
    void UpdateProjectionFromFOV(float32 yoffset);

    glm::vec3 position;
    glm::vec3 lookAt;
    glm::vec3 worldUp;
    
    glm::vec3 upVector;
    glm::vec3 rightVector;
    glm::vec3 frontVector;

    glm::mat4 projectionMatrix;
    glm::mat4 viewMatrix;

    float32 yaw = -90.0f;
    float32 pitch = 0.0f;
    float32 speed = 10.0f;
    float32 sensitivity = 0.1f;

    float32 fov = 45.0f;
    float32 aspect;
    float32 nearPlane = 1.0f;
    float32 farPlane = 100.0f;

private:
    void update_view_matrix();
    void update_projection_matrix();
    void update_normalize_vectors();
};
