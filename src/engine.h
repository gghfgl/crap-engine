#pragma once

enum engine_state {
    ENGINE_ACTIVE,
    ENGINE_MENU,
    ENGINE_TERMINATE
};

enum engine_init_flag {
    NO_FLAG       = 0,
    POLYGONE_MODE = 1 << 0,
    DEBUG_MODE    = 1 << 1,
    VSYNC         = 1 << 2
};

struct engine_time {
    float32 MsPerFrame;
    float32 DeltaTime;
    int64   LastPerfCount;
    int64   PerfCountFrequency;
    int32   FPS;
    int32   MegaCyclePerFrame;
    uint64  LastCycleCount;
};

struct engine {
    engine_state GlobalState;
    const char*  GPUModel;
    const char*  OpenglVersion;
    GLFWwindow  *Window;
    engine_time *Time;

    bool Vsync;
    bool PolyMode;
    bool DebugMode;
};
