#pragma once

struct render_API_info {
    const char*  Vendor;
    const char*  Renderer;
    const char*  Version;
};

struct frame_time {
    float32 MsPerFrame;
    float32 DeltaTime;
    int64   LastPerfCount;
    int64   PerfCountFrequency;
    int32   FPS;
    int32   MegaCyclePerFrame;
    uint64  LastCycleCount;
};

struct window_t {
    int Width, Height;
    render_API_info RenderAPIinfo;
    GLFWwindow *Win32Window;
    const char* WindowTitle;
    frame_time *Time;
    bool Debug = 0;
    bool Vsync;
};
