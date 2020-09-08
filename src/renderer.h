#pragma once

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
