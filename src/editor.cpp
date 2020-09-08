void PrepareAxisDebug(mesh_t *Mesh)
{    
    Mesh->Vertices.clear();

    vertex_t vXa;
    vXa.Position = glm::vec3(0.0f, 0.1f, 0.0f);
    Mesh->Vertices.push_back(vXa);
    vertex_t vXb;
    vXb.Position = glm::vec3(2.0f, 0.1f, 0.0f);
    Mesh->Vertices.push_back(vXb);

    vertex_t vYa;
    vYa.Position = glm::vec3(0.0f, 0.1f, 0.0f);
    Mesh->Vertices.push_back(vYa);
    vertex_t vYb;
    vYb.Position = glm::vec3(0.0f, 2.0f, 0.0f);
    Mesh->Vertices.push_back(vYb);

    vertex_t vZa;
    vZa.Position = glm::vec3(0.0f, 0.1f, 0.0f);
    Mesh->Vertices.push_back(vZa);
    vertex_t vZb;
    vZb.Position = glm::vec3(0.0f, 0.1f, -2.0f);
    Mesh->Vertices.push_back(vZb);
    
    glBindBuffer(GL_ARRAY_BUFFER, Mesh->VBO);
    glBufferSubData(GL_ARRAY_BUFFER,
                    0,
                    Mesh->Vertices.size() * sizeof(vertex_t),
                    &Mesh->Vertices[0]);
}

void PushGridSubData(mesh_t *Mesh, uint32 resolution, uint32 maxResolution)
{
	if (resolution <= maxResolution)
	{
		uint32 vCount = resolution * 4 + 4;			   // 44
		float32 b = (float32)resolution / 2.0f + 1.0f; // 6
		float32 a = -b;								   // -6
		float32 xPos = -((float32)resolution / 2.0f);  // -5
		float32 zPos = xPos;						   // -5

		Mesh->Vertices.clear();
		uint32 i = 0;
		while (i < vCount / 2)
		{
			vertex_t v;
			if (i % 2 == 0)
			{
				v.Position = glm::vec3(a, 0.0f, zPos);
			}
			else
			{
				v.Position = glm::vec3(b, 0.0f, zPos);
				zPos += 1.0f;
			}

			Mesh->Vertices.push_back(v);
			i++;
		}

		while (i < vCount)
		{
			vertex_t v;
			if (i % 2 == 0)
			{
				v.Position = glm::vec3(xPos, 0.0f, a);
			}
			else
			{
				v.Position = glm::vec3(xPos, 0.0f, b);
				xPos += 1.0f;
			}

			Mesh->Vertices.push_back(v);
			i++;
		}
        
		glBindBuffer(GL_ARRAY_BUFFER, Mesh->VBO);
		glBufferSubData(GL_ARRAY_BUFFER,
						0,
						Mesh->Vertices.size() * sizeof(vertex_t),
						&Mesh->Vertices[0]);
	}
}

void PushMouseRaySubData(mesh_t *Mesh, glm::vec3 origin, glm::vec3 direction)
{
	glm::vec3 target = origin + (direction * 1.0f);

	Mesh->Vertices.clear();
	vertex_t v;
	v.Position = glm::vec3(origin.x, origin.y, origin.z - 0.1f);
	Mesh->Vertices.push_back(v);
	v.Position = target;
	Mesh->Vertices.push_back(v);

	glBindBuffer(GL_ARRAY_BUFFER, Mesh->VBO);
	glBufferSubData(GL_ARRAY_BUFFER,
					0,
					Mesh->Vertices.size() * sizeof(vertex_t),
					&Mesh->Vertices[0]);
}
