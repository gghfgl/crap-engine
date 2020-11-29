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
    void updateMouseOffsets();
    float32 getMouseScrollOffsetY();

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
    uint32 getWidth() { return m_width; };
    uint32 getHeight() { return m_height; };
    bool getVsync() { return m_vsync; };
    void updateTime();
    void pollEvents();
    void swapBuffer();
    void toggleVsync();
    
    bool debug;
    GLFWwindow *context;
    FrameTime *time;

private:
    void terminate_window();
    void reset_time();

    uint32 m_width, m_height;
    bool m_vsync;
    const char* m_title;
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