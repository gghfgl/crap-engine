#pragma once

#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "input.h"
#include "renderer.h"
#include "camera.h"

enum engine_state {
    ENGINE_ACTIVE,
    ENGINE_MENU,
    ENGINE_WIN
};

struct engine {
    engine_state GlobalState;	
    input_state *InputState;
    renderer    *Renderer;
    camera      *Camera;
    unsigned int Width, Height;
};
