#pragma once

#define GPU_MAX_JOINT_INFLUENCE 4

struct GPUVertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
    glm::vec3 tangent;
    glm::vec3 bitangent;
    uint32    jointIDs[GPU_MAX_JOINT_INFLUENCE];
    float32   weights[GPU_MAX_JOINT_INFLUENCE];
};

struct GPUTexture
{
    uint32 ID;
    std::string type;
    std::string filename;
};

struct GPUMesh {
    GPUMesh(std::vector<GPUVertex> &vertices, std::vector<uint32> &indices, std::vector<GPUTexture> &textures);
    ~GPUMesh();

    uint32 VAO;
    uint32 VBO;
    uint32 IBO;
    std::vector<GPUVertex> vertices;
    std::vector<uint32> indices;
    std::vector<GPUTexture> textures;
};
