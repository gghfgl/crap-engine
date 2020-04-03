#include <ctime>

#include "entity.h"

entity_cube EntityCubeConstruct(unsigned int id,
				const char* name,
				glm::vec3 position,
				glm::vec3 size,
				float scale,
				glm::vec4 color,
				entity_state state=ENTITY_STATE_STATIC)
{
    entity_cube Entity;
    Entity.ID = id;
    Entity.Name = name;
    Entity.Position = position;
    Entity.Size = size;
    Entity.Scale = scale;
    Entity.Color = color;
    Entity.State = state;

    return Entity;
}

glm::vec3 MouseRayDirectionWorld(float mouseX,float mouseY,
				 int width, int height,
				 glm::mat4 projectionMatrix,
				 glm::mat4 viewMatrix)
{
    // transform to NDC
    float mx = (2.0f * mouseX) / width - 1.0f;
    float my = 1.0f - (2.0f * mouseY) / height;
    float mz = 1.0f;
    glm::vec3 rayNDC = glm::vec3(mx, my, mz);

    glm::vec4 rayClip = glm::vec4(rayNDC.x, rayNDC.y, -1.0, 1.0);
    glm::vec4 rayEye = inverse(projectionMatrix) * rayClip;
    rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0, 0.0);

    glm::vec4 stepWorld = inverse(viewMatrix) * rayEye;
    glm::vec3 rayWorld = glm::vec3(stepWorld.x, stepWorld.y, stepWorld.z);

    return normalize(rayWorld);
}

bool RaySphereIntersection(glm::vec3 rayOriginWorld,
			   glm::vec3 rayDirectionWorld,
			   glm::vec3 sphereCenterWorld,
			   float sphereRadius,
			   float* intersectionDistance)
{
    // work out components of quadratic
    glm::vec3 distToSphere = rayOriginWorld - sphereCenterWorld;
    float b = dot( rayDirectionWorld, distToSphere );
    float c = dot( distToSphere, distToSphere ) - sphereRadius * sphereRadius;
    float b_squared_minus_c = b * b - c;

    // check for "imaginary" answer. == ray completely misses sphere
    if ( b_squared_minus_c < 0.0f ) { return false; }

    // check for ray hitting twice (in and out of the sphere)
    if ( b_squared_minus_c > 0.0f ) {
	// get the 2 intersection distances along ray
	float t_a              = -b + sqrt( b_squared_minus_c );
	float t_b              = -b - sqrt( b_squared_minus_c );
	*intersectionDistance = t_b;

	// if behind viewer, throw one or both away
	if ( t_a < 0.0 ) {
	    if ( t_b < 0.0 ) { return false; }
	} else if ( t_b < 0.0 ) {
	    *intersectionDistance = t_a;
	}

	return true;
    }

    // check for ray hitting once (skimming the surface)
    if ( 0.0f == b_squared_minus_c ) {
	// if behind viewer, throw away
	float t = -b + sqrt( b_squared_minus_c );
	if ( t < 0.0f ) { return false; }
	*intersectionDistance = t;
	return true;
    }

    // note: could also check if ray origin is inside sphere radius
    return false;
}
