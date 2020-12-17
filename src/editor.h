#pragma once

struct editor_t
{
	bool Active;
     Mesh *MeshGrid; // TODO: reference grid
     Mesh *MeshAxisDebug;
     Mesh *MeshRay;
	uint32 GridResolution;
     Skybox *skybox; // TODO: add skybox load from file in panel with min
	bool ShowSkybox;
};

struct GlobalState
{
    bool active = true;
    uint32 currentGroundIndex = 0;
    uint32 currentSkyboxIndex = 0;
    uint32 currentModuleIndex = 0;
    uint32 hoveredModule = 0;
    uint32 selectedModule = 0;
    uint32 dragModule = 0;
};
