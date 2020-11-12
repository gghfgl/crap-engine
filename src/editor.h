#pragma once

struct editor_t
{
	bool Active;
	mesh_t *MeshGrid; // TODO: reference grid
	mesh_t *MeshAxisDebug;
	mesh_t *MeshRay;
	uint32 GridResolution;
	skybox_t *Skybox; // TODO: add skybox load from file in panel with min
	bool ShowSkybox;
};
