#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "input.h"
#include "renderer.h"
#include "camera.h"

enum engine_state {
    ENGINE_ACTIVE,
    ENGINE_MENU,
    ENGINE_TERMINATE
};

enum init_flag {
    NO_DEBUG      = 0x0,
    SHOW_AXIS     = 0x01,
    POLYGONE_MODE = 0x02
};

struct engine {
    engine_state GlobalState;	

    GLFWwindow  *Window;
    input_state *InputState;
    renderer    *Renderer;
    camera      *Camera;
    unsigned int Width, Height;
};
