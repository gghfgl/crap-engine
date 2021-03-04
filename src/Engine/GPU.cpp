#include "GPU.h"

uint32 GPU_allocate_mesh(GPUMesh *mesh)
{
    glGenVertexArrays(1, &mesh->VAO);
    glGenBuffers(1, &mesh->VBO);
    glGenBuffers(1, &mesh->IBO);
  
    glBindVertexArray(mesh->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO);

    if (!mesh->vertices.empty())
    {
        glBufferData(GL_ARRAY_BUFFER,
                     mesh->vertices.size() * sizeof(GPUVertex),
                     &mesh->vertices[0], GL_STATIC_DRAW);  

        // vertex positions
        glEnableVertexAttribArray(0);	
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GPUVertex),
                              (void*)0);

        // vertex normals
        glEnableVertexAttribArray(1);	
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GPUVertex),
                              (void*)offsetof(GPUVertex, normal));

        // vertex texture coords
        glEnableVertexAttribArray(2);	
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(GPUVertex),
                              (void*)offsetof(GPUVertex, texCoords));

        // vertex tangent
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(GPUVertex),
                              (void*)offsetof(GPUVertex, tangent));

        // vertex bitangent
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(GPUVertex),
                              (void*)offsetof(GPUVertex, bitangent));
    }

    if (!mesh->indices.empty())
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->IBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     mesh->indices.size() * sizeof(uint32), 
                     &mesh->indices[0], GL_STATIC_DRAW);
    }
    
    glBindVertexArray(0);
}

void GPU_deallocate_mesh(GPUMesh *mesh)
{
    glDeleteVertexArrays(1, &mesh->VAO);
    glDeleteBuffers(1, &mesh->VBO);
    glDeleteBuffers(1, &mesh->IBO);
}

// =========================================================

GPUMesh::GPUMesh(std::vector<GPUVertex> &vertices, std::vector<uint32> &indices, std::vector<GPUTexture> &textures)
{
    this->vertices = vertices;
    this->indices = indices;
    this->textures = textures;

    GPU_allocate_mesh(this);
}

GPUMesh::~GPUMesh()
{
    // TODO: should be clean after attrib buffer?
    this->vertices.clear();
    this->indices.clear();
    this->textures.clear();

    GPU_deallocate_mesh(this);
}
