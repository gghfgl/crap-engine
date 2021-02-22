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

// Test Ray vs Oriented bounding box
bool RayOBBIntersection(glm::vec3 ray_origin,         // Ray origin, in world space
                            glm::vec3 ray_direction,      // Ray direction (NOT target position!), in world space. Must be normalize()'d.
                            glm::vec3 aabb_min,           // Minimum X,Y,Z coords of the mesh when not transformed at all.
                            glm::vec3 aabb_max,           // Maximum X,Y,Z coords. Often aabb_min*-1 if your mesh is centered, but it's not always the case.
                            glm::mat4 ModelMatrix,        // Transformation applied to the mesh (which will thus be also applied to its bounding box)
                            float32& intersection_distance) // Output : distance between ray_origin and the intersection with the OBB
{
    // Intersection method from Real-Time Rendering and Essential Mathematics for Games
	float tMin = 0.0f;
	float tMax = 100000.0f;

	glm::vec3 OBBposition_worldspace(ModelMatrix[3].x, ModelMatrix[3].y, ModelMatrix[3].z);

	glm::vec3 delta = OBBposition_worldspace - ray_origin;

	// Test intersection with the 2 planes perpendicular to the OBB's X axis
	{
		glm::vec3 xaxis(ModelMatrix[0].x, ModelMatrix[0].y, ModelMatrix[0].z);
		float e = glm::dot(xaxis, delta);
		float f = glm::dot(ray_direction, xaxis);

		if ( fabs(f) > 0.001f ){ // Standard case

			float t1 = (e+aabb_min.x)/f; // Intersection with the "left" plane
			float t2 = (e+aabb_max.x)/f; // Intersection with the "right" plane
			// t1 and t2 now contain distances betwen ray origin and ray-plane intersections

			// We want t1 to represent the nearest intersection, 
			// so if it's not the case, invert t1 and t2
			if (t1>t2){
				float w=t1;t1=t2;t2=w; // swap t1 and t2
			}

			// tMax is the nearest "far" intersection (amongst the X,Y and Z planes pairs)
			if ( t2 < tMax )
				tMax = t2;
			// tMin is the farthest "near" intersection (amongst the X,Y and Z planes pairs)
			if ( t1 > tMin )
				tMin = t1;

			// And here's the trick :
			// If "far" is closer than "near", then there is NO intersection.
			// See the images in the tutorials for the visual explanation.
			if (tMax < tMin )
				return false;

		} else { // Rare case : the ray is almost parallel to the planes, so they don't have any "intersection"
			if(-e+aabb_min.x > 0.0f || -e+aabb_max.x < 0.0f)
				return false;
		}
	}


	// Test intersection with the 2 planes perpendicular to the OBB's Y axis
	// Exactly the same thing than above.
	{
		glm::vec3 yaxis(ModelMatrix[1].x, ModelMatrix[1].y, ModelMatrix[1].z);
		float e = glm::dot(yaxis, delta);
		float f = glm::dot(ray_direction, yaxis);

		if ( fabs(f) > 0.001f ){

			float t1 = (e+aabb_min.y)/f;
			float t2 = (e+aabb_max.y)/f;

			if (t1>t2){float w=t1;t1=t2;t2=w;}

			if ( t2 < tMax )
				tMax = t2;
			if ( t1 > tMin )
				tMin = t1;
			if (tMin > tMax)
				return false;

		} else {
			if(-e+aabb_min.y > 0.0f || -e+aabb_max.y < 0.0f)
				return false;
		}
	}


	// Test intersection with the 2 planes perpendicular to the OBB's Z axis
	// Exactly the same thing than above.
	{
		glm::vec3 zaxis(ModelMatrix[2].x, ModelMatrix[2].y, ModelMatrix[2].z);
		float e = glm::dot(zaxis, delta);
		float f = glm::dot(ray_direction, zaxis);

		if ( fabs(f) > 0.001f ){

			float t1 = (e+aabb_min.z)/f;
			float t2 = (e+aabb_max.z)/f;

			if (t1>t2){float w=t1;t1=t2;t2=w;}

			if ( t2 < tMax )
				tMax = t2;
			if ( t1 > tMin )
				tMin = t1;
			if (tMin > tMax)
				return false;

		} else {
			if(-e+aabb_min.z > 0.0f || -e+aabb_max.z < 0.0f)
				return false;
		}
	}

	intersection_distance = tMin;

	return true;
}
