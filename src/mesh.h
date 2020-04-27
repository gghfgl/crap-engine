#pragma once

struct vertex_t
{
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec3 Tangent;
    glm::vec3 Bitangent;
};

struct texture_t
{
    uint32 Id;
    std::string Type;
    std::string Path;
};

struct mesh_t
{
    uint32 VAO;
    uint32 VBO;
    uint32 IBO;

    std::vector<vertex_t> Vertices;
    std::vector<uint32> Indices;
    std::vector<texture_t> Textures;
};

struct model_t
{
    std::vector<mesh_t*> Meshes;
    std::string Directory;
    std::vector<texture_t> TexturesLoadedCache;
    bool GammaCorrection;
};

struct object_t
{
    const char* Label;
    const char* Filepath;
    model_t *Model;
    mesh_t *PickingSphere;
    glm::vec3 Position;
    float32 Scale = 1.0f;
    float32 Rotate = 0.0f; // degeres
};
