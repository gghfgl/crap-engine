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

// TODO: objectif-> draw without bacthing
// TODO: check batching when repetitive object. (voxel)

// TODO: struct vertex_array (uint32 ID, ARRAY vertex_buffers)
// TODO: struct vertex_buffer (uint32 ID, vertex*)

struct renderer_stats
{
    uint32 DrawCount;
    uint32 CubeCount;
};

// TODO: renderer_t
struct renderer
{
    // TODO: single VAO
    
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

    // TODO: keep memory pool in renderer
    memory_arena *MemoryArena;
    renderer_stats Stats;
};
