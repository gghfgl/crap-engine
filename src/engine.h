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
    double DeltaTime = 0.0f;
    double LastFrameTime = 0.0f;
    double LastFrameTimeFPS = 0.0f;
    int FPS = 0;
    int NextFPS = 0;
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
