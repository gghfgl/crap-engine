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
    Mesh(std::vector<Vertex> vertices,
         std::vector<uint32> indices,
         std::vector<Texture> textures);
    Mesh(float32 margin, float32 radius, uint32 stacks, uint32 slices);
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

    std::vector<Mesh*> Meshes; // TODO: vector of pointers? Meh ....
    std::vector<Texture> TexturesLoadedCache;
    std::string objFilename;
    std::string directory;
    bool gammaCorrection;

private:
    void process_node(aiNode *node, const aiScene *scene);
    Mesh* process_mesh(aiMesh *mesh, const aiScene *scene);
    std::vector<Texture> load_material_textures(aiMaterial *mat,
                                                aiTextureType type,
                                                std::string typeName);
    uint32 load_texture_from_file(const char *path, const std::string &directory);
};

struct Entity
{
    ~Entity();
    
    Model *model; // TODO: why pointer?
    Mesh *pickingSphere;  // TODO: why pointer?
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
    float32 scale = 1.0f;
    float32 rotate = 0.0f; // degres
};

// ======================================

struct Terrain
{
    Terrain(uint32 resolution, glm::vec3 unitSize, std::string const &modelFilePath);
    ~Terrain();
    void updateModelMatrices(uint32 sideLenght);
    
    Entity *entity; // TODO: why pointer?
    uint32 resolution; // default size
    glm::vec3 unitSize;
    uint32 instanceBufferID;
    bool isGenerated = false;
    glm::mat4 *modelMatrices;

private:
    void clear_instance();
};

struct Skybox
{
    Skybox(std::vector<std::string> faces);
    ~Skybox();

    uint32 VAO;
    uint32 VBO;
    uint32 textureID;

private:
    void load_cubemap_texture_from_file(std::vector<std::string> faces);
};
