#pragma once

struct input_state {
    bool   Keyboard[1024];

    float64 MousePosX, MousePosY;
    float32  MouseOffsetX, MouseOffsetY;
    float64 MouseScrollOffsetX, MouseScrollOffsetY;
    float64 MouseLastX, MouseLastY;

    bool   MouseLeftButton;
    bool   MouseLeftButtonFirstClick;
};
