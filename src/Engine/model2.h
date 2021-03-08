#pragma once

// JointTransform represents position / rotation of a joint in joint space.
struct JointTransform {
    glm::vec3 position;
    glm::quat rotation;
};

// Keyframe represents a certain pose at a certain time of the animation.
// @jointTransforms: positions and rotations for each joints related to his parent
// joint. Its in joint space, not model space!
// @timestamp: time to play the pose;
struct Keyframe {
    std::unordered_map<std::string,JointTransform> jointTransforms;
    float32 timestamp;
};

// TODO: useless?? []Keyframe = animation in Animated?
// Animation hold data needed to animate a 3D model.
struct Animation {
    Animation();
    ~Animation();
    
    std::vector<Keyframe> frames;
};

// Animated hold the data needed for animated model.
// @roots: the whole skeleton from tree joint.
struct Animated {
    Animated(const std::string &path);
    ~Animated();
    
    std::string filename;
    std::string directory;
    Joint *rootJoint;
    uint32 jointCount;
    Animation *animation; // TODO: ??
};

// Model represents the minimum stuff for loading 3D model.
// @meshes: the "skin" of the model.
struct Model {
    Model(const std::string &path);
    ~Model();
    
    std::string filename;
    std::string directory;
    std::vector<GPUMesh*> meshes;
};

// TODO: delete
struct BoundingBox
{
    BoundingBox(glm::vec3 maxComponents);
    ~BoundingBox();
    
    uint32 VAO;
    uint32 VBO;
};
