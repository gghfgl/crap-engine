#pragma once

#define GPU_MAX_JOINT_INFLUENCE 4

struct GPUVertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
    glm::vec3 tangent;
    glm::vec3 bitangent;
    int32     jointIDs[GPU_MAX_JOINT_INFLUENCE];
    float32   weights[GPU_MAX_JOINT_INFLUENCE];
};

struct GPUTexture
{
    uint32 ID;
    std::string type;
    std::string filename;
};

struct GPUMesh {
    GPUMesh(std::vector<GPUVertex> &vertices, std::vector<uint32> &indices, std::vector<GPUTexture> &textures, uint32 jointCount);
    ~GPUMesh();

    uint32 VAO;
    uint32 VBO;
    uint32 IBO;
    std::vector<GPUVertex> vertices;
    std::vector<uint32> indices;
    std::vector<GPUTexture> textures;

    uint32 jointCount = 0;
};

// Joint is the tree holding joint hierarchy, starting by the root joint.
// Commonly from the "Hip" to the "Hands" and "Arms".
// @name: name used in the model file used to identify which joint in an
// animation keyframe refer to.
// @transform: current position and rotation of the joint in model-space from the
// binded joint.
// @localBindTransform: original position of the joint relative to it's parent
// in joint-space.
// @inverseBindTransform: is the @localBindTransform in model-space and inverted.
// @children: linked / binded child joints. A joint can hold multiple children.
struct Joint { // TODO: GPUJoint
    // Joint(uint32 ID, std::string name, glm::mat4 localBindTransform); // TODO
    //Joint();
    //~Joint(); // TODO: delete root joint from mesh
    
    uint32 ID;
    std::string name;
    std::vector<Joint> children;

    glm::mat4 animatedTransform;
    glm::mat4 localBindTransform;
    glm::mat4 inverseBindTransform;

    void calc_inverse_bind_transform();
};
