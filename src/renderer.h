#pragma once

#include <glad/glad.h>

#include "shader.h"

static const size_t globalPerCubeVertex  = 8;
static const size_t globalPerCubeIndices = 36;
static const size_t globalMaxCubeCount   = 1000;
static const size_t globalMaxVertexCount = globalMaxCubeCount * globalPerCubeVertex;
static const size_t globalMaxIndexCount  = globalMaxCubeCount * globalPerCubeIndices;

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
