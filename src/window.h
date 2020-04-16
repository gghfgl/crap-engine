#pragma once

struct render_API_info {
    const char*  GPUvendor;
    const char*  Version;
};

struct window_time {
    float32 MsPerFrame;
    float32 DeltaTime;
    int64   LastPerfCount;
    int64   PerfCountFrequency;
    int32   FPS;
    int32   MegaCyclePerFrame;
    uint64  LastCycleCount;
};

struct window_t {
    GLFWwindow  *PlatformWindow; // TODO: using GLFW atm

    int Width, Height;
    render_API_info APIinfo;
    const char*  WindowTitle;
    window_time *Time;
    bool DebugMode;
    bool Vsync;
};
