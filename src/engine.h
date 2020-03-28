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
    DEBUG_MODE    = 0x02
};

struct engine
{
    engine_state GlobalState;
    bool         DebugMode;

    GLFWwindow  *Window;
    input_state *InputState;
    renderer    *Renderer;
    camera      *Camera;
    unsigned int Width, Height;
    unsigned int UBO;
};
