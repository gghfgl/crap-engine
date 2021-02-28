#pragma once

#define MAX_BONE_INFLUENCE 4

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
    glm::vec3 tangent;
    glm::vec3 bitangent;

    // TODO: @animation
    int m_BoneIDs[MAX_BONE_INFLUENCE];    //bone indexes which will influence this vertex
    float m_Weights[MAX_BONE_INFLUENCE];  //weights from each bone
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

struct BoundingBox
{
    BoundingBox(glm::vec3 maxComponents);
    ~BoundingBox();
    
    uint32 VAO;
    uint32 VBO;
};

// TODO: @animation
struct BoneInfo
{
    int id;
    glm::mat4 offset; // offset matrix transforms bone from bone space to local space
};

struct Model
{
    Model(std::string const &path);
    ~Model();

    std::vector<Mesh*> Meshes;
    std::vector<Texture> TexturesLoadedCache;

    glm::vec3 maxComponents = glm::vec3(0.0f, 0.0f, 0.0f);
    BoundingBox *boundingBox;

    std::string objFilename;
    std::string directory;
    bool gammaCorrection;

    // TODO: @animation
    std::map<std::string, BoneInfo> m_OffsetMatMap;
    int32 m_BoneCount = 0;

private:
    void process_node(aiNode *node, const aiScene *scene);
    Mesh* process_mesh(aiMesh *mesh, const aiScene *scene);
    std::vector<Texture> load_material_textures(aiMaterial *mat, aiTextureType type, std::string typeName);
    uint32 load_texture_from_file(const char *path, const std::string &directory);

    // TODO: @animation
    void set_vertex_bone_data_default(Vertex& vertex);
    void set_vertex_bone_data(Vertex& vertex, int boneID, float weight);
    void extract_bone_weight_for_vertices(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene);
};
