#pragma once

#include <glad/glad.h>

#include "shader.h"

#define ASSERT(x) if (!(x)) __debugbreak();
#define GLCall(x) GLClearError();\
    x;\
    ASSERT(GLLogCall(#x, __FILE__, __LINE__))

void GLClearError();
bool GLLogCall(const char* function, const char* file, int line);


struct renderer {
    Shader *shader; //TODO shader refact
    unsigned int cubeVAO;    
};
