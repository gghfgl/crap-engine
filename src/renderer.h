#pragma once

// TODO: think about memory pool after Model loader
/* static const size_t globalPerCubeVertex  = 8; */
/* static const size_t globalPerCubeIndices = 36; */
/* static const size_t globalMaxCubeCount   = 1000; */
/* static const size_t globalMaxVertexCount = globalMaxCubeCount * globalPerCubeVertex; */
/* static const size_t globalMaxIndexCount  = globalMaxCubeCount * globalPerCubeIndices; */

struct renderer_stats
{
    uint32 DrawCalls = 0;
    int64 VertexCount = 0;
};

struct renderer_t
{
    bool WireframeMode;

    renderer_stats Stats;
};

struct vertex_t
{
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec3 Tangent;
    glm::vec3 Bitangent;
    glm::vec4 Color;
};

struct texture_t
{
    uint32 Id;
    std::string Type; // TODO: const char*
    std::string Path; // TODO: const char*
};

struct mesh_t
{
    uint32 VAO;
    uint32 VBO;
    uint32 EBO; // TODO: IBO?

    // array method
    uint32 VertexCount = 0;
    vertex_t *Data = nullptr;
    vertex_t *DataPtr = nullptr;

    // vector method
    std::vector<vertex_t> Vertices;
    std::vector<uint32> Indices;
    std::vector<texture_t> Textures;
};

struct model_t
{
    std::vector<mesh_t*> Meshes; // TODO: free ptr memory!
    std::string Directory; // TODO: const char*
    std::vector<texture_t> TexturesLoaded; // TODO: kind of cache
    bool GammaCorrection;
};
