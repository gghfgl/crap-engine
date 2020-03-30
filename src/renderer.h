#pragma once

#include <glad/glad.h>

#include "shader.h"

static const size_t PerCubeVertex = 8;
static const size_t PerCubeIndices = 36;
static const size_t MaxCubeCount = 500;
static const size_t MaxVertexCount = MaxCubeCount * PerCubeVertex;
static const size_t MaxIndexCount = MaxCubeCount * PerCubeIndices;

struct vertex
{
    glm::vec3 Position;
    glm::vec4 Color;
};

struct renderer_stats
{
    uint32_t DrawCount;
    uint32_t CubeCount;
};

struct renderer
{
    shader *Shader;
    shader *Stencil;

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
