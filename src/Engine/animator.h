#pragma once

uint32 get_joint_position_index(Joint *joint, float32 animationTime)
{
    for (uint32 index = 0; index < joint->numPositions - 1; ++index)
    {
        if (animationTime < joint->positions[index + 1].timestamp)
            return index;
    }
    assert(0);
}

uint32 get_joint_rotation_index(Joint *joint, float32 animationTime)
{
    for (uint32 index = 0; index < joint->numRotations - 1; ++index)
    {
        if (animationTime < joint->rotations[index + 1].timestamp)
            return index;
    }
    assert(0);
}

uint32 get_joint_scale_index(Joint *joint, float32 animationTime)
{
    for (uint32 index = 0; index < joint->numScales - 1; ++index)
    {
        if (animationTime < joint->scales[index + 1].timestamp)
            return index;
    }
    assert(0);
}

float32 get_scale_factor(float32 lastTimeStamp, float32 nextTimeStamp, float32 animationTime)
{
    float32 scaleFactor = 0.0f;
    float32 midWayLength = animationTime - lastTimeStamp;
    float32 framesDiff = nextTimeStamp - lastTimeStamp;
    scaleFactor = midWayLength / framesDiff;
    return scaleFactor;
}

glm::mat4 InterpolatePosition(Joint *joint, float32 animationTime)
{
    if (1 == joint->numPositions)
        return glm::translate(glm::mat4(1.0f), joint->positions[0].position);

    uint32 p0Index = get_joint_position_index(joint, animationTime);
    uint32 p1Index = p0Index + 1;
    float32 scaleFactor = get_scale_factor(joint->positions[p0Index].timestamp,
                                         joint->positions[p1Index].timestamp, animationTime);
    glm::vec3 finalPosition = glm::mix(joint->positions[p0Index].position, joint->positions[p1Index].position
                                       , scaleFactor);
    return glm::translate(glm::mat4(1.0f), finalPosition);
}

glm::mat4 InterpolateRotation(Joint *joint, float32 animationTime)
{
    if (1 == joint->numRotations)
    {
        auto rotation = glm::normalize(joint->rotations[0].orientation);
        return glm::toMat4(rotation);
    }

    uint32 p0Index = get_joint_rotation_index(joint, animationTime);
    uint32 p1Index = p0Index + 1;
    float32 scaleFactor = get_scale_factor(joint->rotations[p0Index].timestamp,
                                       joint->rotations[p1Index].timestamp, animationTime);
    glm::quat finalRotation = glm::slerp(joint->rotations[p0Index].orientation, joint->rotations[p1Index].orientation
                                         , scaleFactor);
    finalRotation = glm::normalize(finalRotation);
    return glm::toMat4(finalRotation);

}

glm::mat4 InterpolateScaling(Joint *joint, float32 animationTime)
{
    if (1 == joint->numScales)
        return glm::scale(glm::mat4(1.0f), joint->scales[0].scale);

    uint32 p0Index = get_joint_scale_index(joint, animationTime);
    uint32 p1Index = p0Index + 1;
    float32 scaleFactor = get_scale_factor(joint->scales[p0Index].timestamp,
                                       joint->scales[p1Index].timestamp, animationTime);
    glm::vec3 finalScale = glm::mix(joint->scales[p0Index].scale, joint->scales[p1Index].scale
                                    , scaleFactor);
    return glm::scale(glm::mat4(1.0f), finalScale);
}

void update_joint_transform(Joint *joint, float32 animationTime)
{
    glm::mat4 translation = InterpolatePosition(joint, animationTime);
    glm::mat4 rotation = InterpolateRotation(joint, animationTime);
    glm::mat4 scale = InterpolateScaling(joint, animationTime);
    joint->localTransform = translation * rotation * scale;
}

struct Animator
{
    Animation *currentAnimation;
    std::vector<glm::mat4> transforms;
    float32 currentTime; // @TODO: timestamp?
    float32 deltaTime; // @TODO: meh.. ?

    // =======================================================

    // @TODO: rework this crap
    Animator(Animation* animation)
        {
            this->currentAnimation = animation;
            this->currentTime = 0.0;
            this->transforms.reserve(MAX_JOINTS);
            for (int i = 0; i < MAX_JOINTS; i++)
                this->transforms.push_back(glm::mat4(1.0f));
        }

    void UpdateAnimation(float32 deltaTime)
        {
            this->deltaTime = deltaTime;
            if (this->currentAnimation)
            {
                this->currentTime += this->currentAnimation->ticksPerSecond * deltaTime;
                this->currentTime = fmod(this->currentTime, this->currentAnimation->duration);
                CalculateBoneTransform(&this->currentAnimation->rootNode, glm::mat4(1.0f));
            }
        }

    void CalculateBoneTransform(const AnimationNode* node, glm::mat4 parentTransform)
        {
            // DEBUG
            Log::debug("compute bone transform\n");
            std::string nodeName = node->name;
            glm::mat4 nodeTransform = node->transformation;

            Joint* joint = this->currentAnimation->FindBone(nodeName);

            if (joint)
            {
                update_joint_transform(joint, this->currentTime);
                nodeTransform = joint->localTransform;
            }

            glm::mat4 globalTransformation = parentTransform * nodeTransform;

            auto jointTransforms = this->currentAnimation->jointTransforms;
            if (jointTransforms.find(nodeName) != jointTransforms.end())
            {
                uint32 index = jointTransforms[nodeName].ID;
                glm::mat4 offset = jointTransforms[nodeName].localTransform;
                this->transforms[index] = globalTransformation * offset;
            }

            for (int i = 0; i < node->childrenCount; i++)
                CalculateBoneTransform(&node->children[i], globalTransformation);
        }    
};
