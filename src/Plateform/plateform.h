#pragma once

#include "keyboard.h"

struct KeyboardEvent
{
    bool isPressed[512] = {0};
    bool isReleased[512] = {0};
};

struct MouseEvent
{
    float64  posX, posY;
    float64  lastX, lastY;
    float32  offsetX, offsetY;
    float64  scrollOffsetX, scrollOffsetY;
    bool     leftButton;
    bool     leftButtonFirstClick;  
    bool     rightButton;
    bool     rightButtonFirstClick;  
};

struct InputState
{
    ~InputState();
    void updateMouseOffsets();
    float32 getMouseScrollOffsetY();

    KeyboardEvent *KeyboardEvent;
    MouseEvent    *MouseEvent;
};

struct RenderInfoAPI {
    const char*  vendor;
    const char*  renderer;
    const char*  version;
};

struct FrameTime {
    float32 msPerFrame;
    float32 deltaTime;
    int64   lastPerfCount;
    int64   perfCountFrequency;
    int32   fps;
    int32   megaCyclePerFrame;
    uint64  lastCycleCount;
};

struct WindowWrapper {
    WindowWrapper(uint32 t_width, uint32 t_height, const char *t_title);
    ~WindowWrapper();
    uint32 getWidth() { return m_width; };
    uint32 getHeight() { return m_height; };
    bool getVsync() { return m_vsync; };
    void updateTime();
    void pollEvents();
    void swapBuffer();
    void toggleVsync();
    
    bool debug;
    GLFWwindow *Context;
    FrameTime *Time;

private:
    void terminate_window();
    void set_time();

    uint32 m_width, m_height;
    bool m_vsync;
    const char* m_title;
};

struct Plateform {
    Plateform(uint32 t_width, uint32 t_height, const char *t_title);
    ~Plateform();

    RenderInfoAPI RenderAPI;
    WindowWrapper *Window;
    InputState *Input;

private:
    void bind_input();
};
