#pragma once

// TODO: think about memory pool after Model loader
/* static const size_t globalPerCubeVertex  = 8; */
/* static const size_t globalPerCubeIndices = 36; */
/* static const size_t globalMaxCubeCount   = 1000; */
/* static const size_t globalMaxVertexCount = globalMaxCubeCount * globalPerCubeVertex; */
/* static const size_t globalMaxIndexCount  = globalMaxCubeCount * globalPerCubeIndices; */

struct renderer_stats
{
    uint32 DrawCalls = 0;
    int64 VertexCount = 0;
};

struct renderer_t
{
    bool WireframeMode;

    renderer_stats Stats;
};
