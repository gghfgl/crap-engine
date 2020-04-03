#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

const float globalYawSetting         = -90.0f;
const float globalPitchSetting       =  0.0f;
const float globalSpeedSetting       =  10.0f;
const float globalSensitivitySetting =  0.1f;
const float globalFovSetting         =  45.0f;

enum camera_movement
{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

struct camera
{
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    // Euler Angles
    float Yaw;
    float Pitch;

    float MovementSpeed;
    float MouseSensitivity;
    float Fov;
};
