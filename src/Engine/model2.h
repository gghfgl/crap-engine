#pragma once

// This should be the maximum bone<->weights relation
#define MAX_JOINT_INFLUENCE 4

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
    glm::vec3 tangent;
    glm::vec3 bitangent;
    int32     jointIDs[MAX_JOINT_INFLUENCE];
    float32   weights[MAX_JOINT_INFLUENCE];
};

struct Texture
{
    uint32 ID;
    std::string type;
    std::string filename;
};

struct Joint {
    uint32 ID;
    glm::mat4 localTransform;
};

struct Mesh {
    Mesh(std::vector<Vertex> &vertices, std::vector<uint32> &indices, std::vector<Texture> &textures);
    ~Mesh();

    uint32 VAO;
    uint32 VBO;
    uint32 IBO;

    std::vector<Vertex> vertices;
    std::vector<uint32> indices;
    std::vector<Texture> textures;
};

/* struct JointTransform { */
/*     glm::vec3 position; */
/*     glm::quat rotation; */
/* }; */

/* struct Keyframe { */
/*     std::unordered_map<std::string,JointTransform> jointTransforms; */
/*     float32 timestamp; */
/* }; */

/* struct Animation { */
/*     Animation(); */
/*     ~Animation(); */
    
/*     std::vector<Keyframe> frames; */
/* }; */

/* struct Animated { */
/*     Animated(const std::string &path); */
/*     ~Animated(); */
    
/*     std::string filename; */
/*     std::string directory; */

/*     Animation *animation; // TODO: useless []Keyframe = animation in Animated? */
/* }; */

struct Model {
    Model(const std::string &path);
    ~Model();

    std::string filename;
    std::string directory;

    std::vector<Mesh*> meshes;                    // Skin
    std::unordered_map<std::string,Joint> joints; // Bones
    uint32 jointCount;
};

// TODO: delete
struct BoundingBox
{
    BoundingBox(glm::vec3 maxComponents);
    ~BoundingBox();
    
    uint32 VAO;
    uint32 VBO;
};
