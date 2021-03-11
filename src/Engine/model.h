#pragma once

#define MAX_JOINTS 100 // This should be the maximum of joints from a single model
#define MAX_JOINT_INFLUENCE 4 // This should be the maximum bone<->weights relation

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

struct KeyPosition
{
    glm::vec3 position;
    float32 timestamp;
};

struct KeyRotation
{
    glm::quat orientation;
    float32 timestamp;
};

struct KeyScale
{
    glm::vec3 scale;
    float32 timestamp;
};

struct Joint {
    Joint(uint32 ID, std::string name);
    ~Joint();

    uint32 ID;
    std::string name;
    glm::mat4 localTransform = glm::mat4(1.0f);

    std::vector<KeyPosition> positions;
    std::vector<KeyRotation> rotations;
    std::vector<KeyScale> scales;

    uint32 numPositions;
    uint32 numRotations;
    uint32 numScales;
};

struct JointTransform {
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

struct AnimationNode
{
	std::string name;
	int32 childrenCount;
	glm::mat4 transformation;
	std::vector<AnimationNode> children;
};

struct Animation {
    ~Animation();

    // @TODO: clean this crap
    Joint* FindBone(const std::string& name)
        {
            auto iter = std::find_if(joints.begin(), joints.end(),
                                     [&](Joint *joint)
                                     {
                                         return joint->name == name;
                                     }
                );
            if (iter == joints.end()) return nullptr;
            else return *iter;
        }
    
    std::string filename;
    std::string directory;

    float32 duration = 0;
    uint32 ticksPerSecond = 0;
    AnimationNode rootNode;

    std::vector<Joint*> joints; // @Warning: loaded during load_animation().
    std::unordered_map<std::string,JointTransform> jointTransforms; // @Warning: loaded during load_model().
    uint32 jointCount;
};

struct Model {
    Model(const std::string &path);
    ~Model();
    void LoadAnimation(const std::string &path);

    std::string filename;
    std::string directory;

    Animation *animation;
    std::vector<Mesh*> meshes;
};

// TODO: delete
struct BoundingBox
{
    BoundingBox(glm::vec3 maxComponents);
    ~BoundingBox();
    
    uint32 VAO;
    uint32 VBO;
};
