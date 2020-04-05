#pragma once

struct input_state {
    bool   Keyboard[1024];

    double MousePosX, MousePosY;
    float  MouseOffsetX, MouseOffsetY;
    double MouseScrollOffsetX, MouseScrollOffsetY;
    double MouseLastX, MouseLastY;

    bool   MouseLeftButton;
    bool   MouseLeftButtonFirstClick;
};
