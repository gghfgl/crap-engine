#pragma once

#include <GLFW/glfw3.h>

struct input_state {
    GLFWwindow* Window;
    bool   Keyboard[1024];

    double MousePosX, MousePosY;
    float  MouseOffsetX, MouseOffsetY;
    double MouseScrollOffsetX, MouseScrollOffsetY;
    double MouseLastX, MouseLastY;

    bool   MouseLeftButton;
    bool   MouseLeftButtonFirstClick;
};
