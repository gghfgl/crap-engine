#pragma once

static const float32 g_DefaultYawSetting         = -90.0f;
static const float32 g_DefaultPitchSetting       =  0.0f;
static const float32 g_DefaultSpeedSetting       =  10.0f;
static const float32 g_DefaultSensitivitySetting =  0.1f;
static const float32 g_DefaultFovSetting         =  45.0f;

enum camera_movement
{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

struct camera_settings
{
    float32 Yaw;
    float32 Pitch;
    float32 Speed;
    float32 Sensitivity;
    float32 Fov;    
};

struct camera_t
{
    camera_settings *Settings;

    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    glm::mat4 ProjectionMatrix;
};
