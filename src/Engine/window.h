#pragma once

struct render_API_info {
    const char*  Vendor;
    const char*  Renderer;
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
    int Width, Height;
    render_API_info APIinfo;
    GLFWwindow *PlatformWindow; // TODO: using GLFW atm
    const char* WindowTitle;
    window_time *Time;
    bool DebugMode = 0;
    bool Vsync;
};
