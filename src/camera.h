#pragma once

const float globalDefaultYawSetting         = -90.0f;
const float globalDefaultPitchSetting       =  0.0f;
const float globalDefaultSpeedSetting       =  10.0f;
const float globalDefaultSensitivitySetting =  0.1f;
const float globalDefaultFovSetting         =  45.0f;

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
    float Yaw;
    float Pitch;
    float Speed;
    float Sensitivity;
    float Fov;    
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
