#pragma once

#include "keyboard.h"

struct KeyboardEvent
{
    bool isPressed[512] = {0};
    bool isReleased[512] = {0};
};

struct MouseEvent
{
    float64 posX, posY;
    float64 lastX, lastY;
    float32 offsetX, offsetY;
    float64 scrollOffsetX, scrollOffsetY;
    bool leftButton;
    bool leftButtonFirstClick;  
    bool rightButton;
    bool rightButtonFirstClick;  
};

struct InputState
{
    ~InputState();
    void UpdateMouseOffsets();
    float32 GetMouseScrollOffsetY();

    KeyboardEvent *keyboard;
    MouseEvent    *mouse;
};

struct FrameTime {
    float32 msPerFrame;
    float32 deltaTime;
    float32 lastFrame;
    int32   fps;
};

struct Window {
    Window(uint32 width, uint32 height, const char *title);
    ~Window();
    uint32 GetWidth() { return width; };
    uint32 GetHeight() { return height; };
    bool GetVsync() { return vsync; };
    void UpdateTime();
    void PollEvents();
    void SwapBuffer();
    void ToggleVsync();
    
    bool debug;
    GLFWwindow *context;
    FrameTime *time;

private:
    void terminate_window();
    void reset_time();

    uint32 width, height;
    bool vsync;
    const char* title;
};

struct PlateformInfo
{
    const char* vendor;
    const char* graphicAPI;
    const char* versionAPI;
};

struct Plateform {
    Plateform(uint32 width, uint32 height, const char *title);
    ~Plateform();

    Window *window;
    InputState *input;
    PlateformInfo *info;

private:
    void bind_input();
};
