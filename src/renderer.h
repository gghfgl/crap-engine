#pragma once

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
    glm::vec2 TextureCoord;
    float32 TextureIndex;
};

struct renderer_stats
{
    uint32 DrawCount;
    uint32 CubeCount;
};

struct renderer
{
    // NOTE: renderer data ...
    uint32 DebugVAO = 0;
    uint32 DebugVBO = 0;
    uint32 CubeVAO = 0;
    uint32 CubeVBO = 0;
    uint32 CubeIBO = 0;
    uint32 IndexCount = 0;
    vertex *CubeBuffer = nullptr;
    vertex *CubeBufferPtr = nullptr;

    uint32 DefaultTextureID; // TODO: meh ...

    memory_arena *MemoryArena;
    renderer_stats Stats;
};

/* struct rendere_memory_pool */
/* { */
/*     vertex *CubeBuffer; */
/* }; */
