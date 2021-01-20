#pragma once

// TODO: enum draw option
// DRAW_MODULES
// DRAW_ENVIRONMENTS

struct GlobalState
{
    bool active = true;
    uint32 currentGroundIndex = 0; // TODO: rename to selectedGround
    uint32 currentSkyboxIndex = 0; // TODO: rename to selectedSkybox
    uint32 currentModuleIndex = 0; // TODO: use this for outline selection?
    uint32 hoveredModule = 0;
    uint32 selectedModule = 0;
    uint32 dragModule = 0;
    std::map<uint32, Module*> *selectedModules;

    // TODO: switch to enum
    bool drawModules = false;
};
