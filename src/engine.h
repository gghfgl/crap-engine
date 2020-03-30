#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "input.h"
#include "renderer.h"
#include "camera.h"

enum engine_state
{
    ENGINE_ACTIVE,
    ENGINE_MENU,
    ENGINE_TERMINATE
};

enum init_flag
{
    NO_FLAG       = 0x0,
    POLYGONE_MODE = 0x01,
    DEBUG_MODE    = 0x02,
    VSYNC         = 0x08
};

struct engine_time
{
    double DeltaTime = 0.0f;
    double LastFrameTime = 0.0f;
    double LastFrameTimeFPS = 0.0f;
    int FPS = 0;
    int NextFPS = 0;
};

struct engine
{
    engine_state GlobalState;
    bool         DebugMode;

    engine_time *Time;

    GLFWwindow  *Window;
    input_state *InputState;
    renderer    *Renderer;
    camera      *Camera;
    unsigned int Width, Height;
    unsigned int UBO;
};
