#pragma once

enum entity_state
{
    ENTITY_STATIC,
    ENTITY_DYNAMIC,
    ENTITY_SLOT
};

struct entity_cube
{
    unsigned int ID;
    glm::vec3 Position;
    glm::vec4 Size; // TODO: move scale to single float
    glm::vec4 Color;
    bool IsSelected = false;
    entity_state State;
};
