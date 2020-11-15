#pragma once

static const float32 g_DefaultYawSetting         = -90.0f;
static const float32 g_DefaultPitchSetting       =  0.0f;
static const float32 g_DefaultSpeedSetting       =  10.0f;
static const float32 g_DefaultSensitivitySetting =  0.1f;
static const float32 g_DefaultFovSetting         =  45.0f;

enum CameraDirection
{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

struct CameraSetting
{
    float32 yaw;
    float32 pitch;
    float32 speed;
    float32 sensitivity;
    float32 fov;    
};

struct Camera {
    Camera(float32 windowWidth, float32 windowHeight,
           glm::vec3 position, glm::vec3 worldUp,
           float32 yaw, float32 pitch, float32 speed, float32 sensitivity, float32 fov,
           float32 nearPlane, float32 farPlane);
    ~Camera();
    glm::mat4 getViewMatrix();
    void processMovementDirection(CameraDirection direction,
                                  float32 deltaTime,
                                  float32 acceleration);
    void processMovementAngles(float32 xoffset,
                               float32 yoffset,
                               bool constrainPitch);
    void processMovementFov(float32 yoffset);
    
    CameraSetting *Settings;
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp;
    glm::mat4 projectionMatrix;

private:
    void update_camera_vectors();
};
