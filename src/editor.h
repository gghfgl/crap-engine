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
    uint32 g_CurrentGroundIndex = 0;
    uint32 g_CurrentSkyboxIndex = 0;
    uint32 g_CurrentModuleIndex = 0;
    uint32 g_HoveredModule = 0;
    uint32 g_SelectedModule = 0;
    uint32 g_DragModule = 0;
};
