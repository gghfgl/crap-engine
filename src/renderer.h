#pragma once

#include <glad/glad.h>

#include "shader.h"

#define ASSERT(x) if (!(x)) __debugbreak();
#define GLCall(x) GLClearError();\
    x;\
    ASSERT(GLLogCall(#x, __FILE__, __LINE__))

void GLClearError();
bool GLLogCall(const char* function, const char* file, int line);

static const size_t PerCubeVertex = 8;
static const size_t PerCubeIndices = 36;
static const size_t MaxCubeCount = 200;
static const size_t MaxVertexCount = MaxCubeCount * PerCubeVertex;
static const size_t MaxIndexCount = MaxCubeCount * PerCubeIndices;

struct vertex {
    glm::vec3 Position;
    glm::vec4 Color;
};

struct renderer_stats {
    uint32_t DrawCount;
    uint32_t CubeCount;
};

struct renderer {
    shader *Shader;

    unsigned int DebugVAO = 0;
    unsigned int DebugVBO = 0;

    unsigned int CubeVAO = 0;
    unsigned int CubeVBO = 0;
    unsigned int CubeIBO = 0;

    uint32_t IndexCount = 0;

    vertex *CubeBuffer = nullptr;
    vertex *CubeBufferPtr = nullptr;

    renderer_stats Stats;
};
