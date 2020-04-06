#pragma once

const float32 globalDefaultYawSetting         = -90.0f;
const float32 globalDefaultPitchSetting       =  0.0f;
const float32 globalDefaultSpeedSetting       =  10.0f;
const float32 globalDefaultSensitivitySetting =  0.1f;
const float32 globalDefaultFovSetting         =  45.0f;

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

struct camera
{
    camera_settings *Settings;

    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    glm::mat4 ProjectionMatrix;
};
