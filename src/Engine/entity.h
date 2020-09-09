#pragma once

#include "mesh.h"

struct entity_t
{
    model_t *Model;
    mesh_t *PickingSphere;
    glm::vec3 Position = glm::vec3(0.0f, 0.0f, 0.0f);
    float32 Scale = 1.0f;
    float32 Rotate = 0.0f; // degres
};

// ======================================

struct terrain_t
{
    entity_t *Entity;
    uint32 SideLenght = 6; // default size
    bool IsGenerated = false;
    glm::mat4 *ModelMatrices;
};

struct skybox_t
{
    uint32 VAO;
    uint32 VBO;
    uint32 TextureId;
};
