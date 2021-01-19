#pragma once

struct GlobalState
{
    bool active = true;
    uint32 currentGroundIndex = 0;
    uint32 currentSkyboxIndex = 0;
    uint32 currentModuleIndex = 0;
    uint32 hoveredModule = 0;
    uint32 selectedModule = 0;
    uint32 dragModule = 0;
    bool drawModules = false;
};
