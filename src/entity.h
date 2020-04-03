#pragma once

enum entity_state
{
    ENTITY_STATE_STATIC,
    ENTITY_STATE_DYNAMIC,
    ENTITY_STATE_SLOT
};

struct entity_cube
{
    unsigned int ID;
    glm::vec3 Position;
    glm::vec3 Size;
    float Scale;
    glm::vec4 Color;
    bool IsSelected = false;
    entity_state State;
};
