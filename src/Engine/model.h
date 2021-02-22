#pragma once

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
    glm::vec3 tangent;
    glm::vec3 bitangent;
};

struct Texture
{
    uint32 ID;
    std::string type;
    std::string path;
};

struct Mesh
{
    Mesh(std::vector<Vertex> vertices, std::vector<uint32> indices, std::vector<Texture> textures);
    //Mesh(float32 margin, float32 radius, uint32 stacks, uint32 slices);
    ~Mesh();
    
    uint32 VAO;
    uint32 VBO;
    uint32 IBO;

    std::vector<Vertex> Vertices;
    std::vector<uint32> Indices;
    std::vector<Texture> Textures;

private:
    void allocate_mesh();
};

struct Model
{
    Model(std::string const &path);
    ~Model();

    std::vector<Mesh*> Meshes;
    std::vector<Texture> TexturesLoadedCache;

    std::string objFilename;
    std::string directory;
    bool gammaCorrection;

private:
    void process_node(aiNode *node, const aiScene *scene);
    Mesh* process_mesh(aiMesh *mesh, const aiScene *scene);
    std::vector<Texture> load_material_textures(aiMaterial *mat, aiTextureType type, std::string typeName);
    uint32 load_texture_from_file(const char *path, const std::string &directory);
};
