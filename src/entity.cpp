#include <ctime>

#include "entity.h"

unsigned int GetIndexByColor(int r, int g, int b)
{
    return (r)|(g<<8)|(b<<16);
}

entity_cube EntityCubeConstruct(unsigned int id,
			     glm::vec3 position,
			     glm::vec4 size,
			     glm::vec4 color,
			     entity_state state=ENTITY_STATIC)
{
    entity_cube Entity;
    // Entity.ID = (int)time(0);
    // float r = (color.r >= 1.0f ? 255 : color.r * 255); // TODO: bad ... 256 ?
    // float g = (color.g >= 1.0f ? 255 : color.g * 255); // TODO: bad ... 256 ?
    // float b = (color.b >= 1.0f ? 255 : color.b * 255); // TODO: bad ... 256 ?
    // Entity.ID = GetIndexByColor((int)r, (int)g, (int)b);
    Entity.ID = id;
    Entity.Position = position;
    Entity.Size = size;
    Entity.Color = color;
    Entity.State = state;

    return Entity;
}
