#pragma once

enum entity_state
{
    ENTITY_STATIC,
    ENTITY_DYNAMIC
};

struct entity_cube
{
    unsigned int ID;
    glm::vec3 Position;
    glm::vec4 Size; // with scale
    glm::vec4 Color;
    bool IsSelected = false;
    entity_state State = ENTITY_STATIC;
};
