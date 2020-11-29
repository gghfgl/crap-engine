glm::vec3 MouseRayDirectionWorld(float32 mouseX,float32 mouseY,
				 int width, int height,
				 glm::mat4 projectionMatrix,
				 glm::mat4 viewMatrix)
{
    // transform to NDC
    float32 mx = (2.0f * mouseX) / (float32)width - 1.0f;
    float32 my = 1.0f - (2.0f * mouseY) / (float32)height;
    float32 mz = 1.0f;
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
						   float32 sphereRadius,
						   float32 *intersectionDistance)
{
	// work out components of quadratic
	glm::vec3 distToSphere = rayOriginWorld - sphereCenterWorld;
	float32 b = dot(rayDirectionWorld, distToSphere);
	float32 c = dot(distToSphere, distToSphere) - sphereRadius * sphereRadius;
	float32 b_squared_minus_c = b * b - c;

	// check for "imaginary" answer. == ray completely misses sphere
	if (b_squared_minus_c < 0.0f)
	{
		return false;
	}

	// check for ray hitting twice (in and out of the sphere)
	if (b_squared_minus_c > 0.0f)
	{
		// get the 2 intersection distances along ray
		float32 t_a = -b + sqrt(b_squared_minus_c);
		float32 t_b = -b - sqrt(b_squared_minus_c);
		*intersectionDistance = t_b;

		// if behind viewer, throw one or both away
		if (t_a < 0.0)
		{
			if (t_b < 0.0)
			{
				return false;
			}
		}
		else if (t_b < 0.0)
		{
			*intersectionDistance = t_a;
		}

		return true;
	}

	// check for ray hitting once (skimming the surface)
	if (0.0f == b_squared_minus_c)
	{
		// if behind viewer, throw away
		float32 t = -b + sqrt(b_squared_minus_c);
		if (t < 0.0f)
		{
			return false;
		}
		*intersectionDistance = t;
		return true;
	}

	// note: could also check if ray origin is inside sphere radius
	return false;
}

// In case of plane (infint plane surface) it will be always intersec until the ray is parallel to the plane
bool RayPlaneIntersection(glm::vec3 rayOriginWorld,
						  glm::vec3 rayDirectionWorld, // Normalized !!!!
						  glm::vec3 planeCoord,
						  glm::vec3 planeNormal,
						  glm::vec3 *intersectionPoint)
{
	/*
      What does the d value mean ?
      For two vectors a and b a dot product actually returns the length of the orthogonal projection of one vector on the other times this other vector.
      But if a is normalized (length = 1), Dot(a, b) is then the length of the projection of b on a. In case of our plane, d gives us the directional distance all points of the plane in the normal direction to the origin (a is the normal). We can then get whether a point is on this plane by comparing the length of the projection on the normal (Dot product).
    */
	float32 d = dot(planeNormal, planeCoord);

	if (dot(planeNormal, rayDirectionWorld) == 0)
	{
		return false; // No intersection, the line is parallel to the plane
	}

	// Compute the X value for the directed line ray intersecting the plane
	float32 x = (d - dot(planeNormal, rayOriginWorld)) / dot(planeNormal, rayDirectionWorld);

	// output itersec point
	*intersectionPoint = rayOriginWorld + rayDirectionWorld * x;
	return true;
}
