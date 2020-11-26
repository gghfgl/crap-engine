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

struct Environment
{
    std::string name;
    Ground *ground;
    int32 *sliderResolution;
    Skybox *skybox;
};
