#include "model2.h"

void GPU_delete_mesh(Mesh *mesh)
{
    glDeleteVertexArrays(1, &mesh->VAO);
    glDeleteBuffers(1, &mesh->VBO);
    glDeleteBuffers(1, &mesh->IBO);
}

uint32 GPU_allocate_mesh(Mesh *mesh)
{
    glGenVertexArrays(1, &mesh->VAO);
    glGenBuffers(1, &mesh->VBO);
    glGenBuffers(1, &mesh->IBO);
  
    glBindVertexArray(mesh->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO);

    if (!mesh->vertices.empty())
    {
        glBufferData(GL_ARRAY_BUFFER,
                     mesh->vertices.size() * sizeof(Vertex),
                     &mesh->vertices[0], GL_STATIC_DRAW);  

        // vertex positions
        glEnableVertexAttribArray(0);	
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                              (void*)0);

        // vertex normals
        glEnableVertexAttribArray(1);	
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                              (void*)offsetof(Vertex, normal));

        // vertex texture coords
        glEnableVertexAttribArray(2);	
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                              (void*)offsetof(Vertex, texCoords));

        // vertex tangent
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                              (void*)offsetof(Vertex, tangent));

        // vertex bitangent
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                              (void*)offsetof(Vertex, bitangent));
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

// =================================

Mesh::Mesh(std::vector<Vertex> &vertices, std::vector<uint32> &indices, std::vector<Texture> &textures)
{
    this->vertices = vertices;
    this->indices = indices;
    this->textures = textures;

    GPU_allocate_mesh(this);
}

Mesh::~Mesh()
{
    GPU_delete_mesh(this);

    // TODO: should be clean after attrib buffer?
    this->vertices.clear();
    this->indices.clear();
    this->textures.clear();
}

Model::Model(const std::string &path)
{
    Log::separator();
    Log::info("loading model: '%s'\n", path.c_str());

    // TODO: wrap in internal? is it really relative path?
    // convert to relative path.
    auto p = std::filesystem::proximate(path);
    std::string p_string{p.u8string()};

    this->directory = p_string.substr(0, p_string.find_last_of('/'));
    this->filename = p_string.substr(this->directory.length() + 1, p_string.length());

    std::string error = ASSET_load_meshes(this->directory, this->filename, this->meshes);
    if (error != "")
    {
        this->directory = "";
        this->filename = "";

        Log::error("asset_load_model: '%s'\n", error.c_str());
        Log::separator();
        return;
    }


    Log::info("done!\n");
    Log::separator();
}

Model::~Model()
{
    for (auto& m : this->meshes)
        delete m;
    this->meshes.clear();
}
