#include <ctime>

#include "entity.h"

entity_cube EntityCubeConstruct(unsigned int id,
				glm::vec3 position,
				glm::vec3 size,
				float scale,
				glm::vec4 color,
				entity_state state=ENTITY_STATE_STATIC)
{
    entity_cube Entity;
    Entity.ID = id;
    Entity.Position = position;
    Entity.Size = size;
    Entity.Scale = scale;
    Entity.Color = color;
    Entity.State = state;

    return Entity;
}
