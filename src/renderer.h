#pragma once

#include "shader.h"

// TODO: think about memory pool after Model loader
/* static const size_t globalPerCubeVertex  = 8; */
/* static const size_t globalPerCubeIndices = 36; */
/* static const size_t globalMaxCubeCount   = 1000; */
/* static const size_t globalMaxVertexCount = globalMaxCubeCount * globalPerCubeVertex; */
/* static const size_t globalMaxIndexCount  = globalMaxCubeCount * globalPerCubeIndices; */

struct vertex
{
    glm::vec3 Position;
    glm::vec4 Color;
    glm::vec2 TextureCoord;
    float32 TextureIndex;
};

struct vertex_buffer
{
    uint32 ID;
    vertex *Buffer = nullptr;
    vertex *BufferPtr = nullptr;
};

struct vertex_array
{
    uint32 ID;
    vertex_buffer *VBOs = nullptr;
    vertex_buffer *VBOsPtr = nullptr;
};

struct renderer_stats
{
    uint32 DrawCalls;
    int64 VertexCount;
};

struct renderer_t
{
    bool PolygoneMode;
    vertex_array *VAO = nullptr;

    renderer_stats Stats;
    memory_pool *MemPool; // TODO: do something with this?
};
