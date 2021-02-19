#pragma once

enum DRAW_FILTER {
    MODULES_FILTER = 1,
    ENVIRONMENT_FILTER = 2
};

struct EditorState
{
    DRAW_FILTER drawFilter = MODULES_FILTER;
    std::map<uint32, Module*> *selectedModules;
    uint32 currentGroundIndex = 0;
    uint32 currentSkyboxIndex = 0;
    uint32 hoveredModule = 0;
    uint32 selectedModuleIndex = 0;
    uint32 dragModule = 0;

};
