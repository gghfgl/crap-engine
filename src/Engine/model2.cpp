#include "model2.h"

// ASSET
std::string load_model(Model *model);
void process_node(Model *model, const aiScene *scene, aiNode *node);
Mesh* process_mesh(Model *model, aiMesh *mesh, const aiScene *scene);
void process_joint_data(Model *model, std::vector<Vertex> &vertices, aiMesh* mesh);
std::vector<Texture> load_material_textures(const std::string &directory, aiMaterial *mat, aiTextureType type, std::string typeName);
uint32 load_texture_from_file(const std::string &path);
std::string load_animation(Animation *animation, Model *model);
void process_animation_hierarchy(AnimationNode& dest, const aiNode* src);
void setup_animation_joints(const aiAnimation* aiAnim, Model *model);
Joint* create_animation_joint(const std::string& name, int32 ID, const aiNodeAnim* channel);

glm::mat4 convert_aimatrix_to_glm(const aiMatrix4x4& from);
glm::vec3 convert_aivec_to_glm(const aiVector3D& vec);
glm::quat convert_aiquat_to_glm(const aiQuaternion& pOrientation);

// GPU
void allocate_mesh(Mesh *mesh);
void deallocate_mesh(Mesh *mesh);

// ======================================================================================

// TODO
Joint::Joint(uint32 ID, std::string name)
{
    this->ID = ID;
    this->name = name;
}

// TODO
Joint::~Joint()
{
    this->positions.clear();
    this->rotations.clear();
    this->scales.clear();
}

Mesh::Mesh(std::vector<Vertex> &vertices, std::vector<uint32> &indices, std::vector<Texture> &textures)
{
    this->vertices = vertices;
    this->indices = indices;
    this->textures = textures;

    allocate_mesh(this);
}

Mesh::~Mesh()
{
    // TODO: should be clean after attrib buffer?
    this->vertices.clear();
    this->indices.clear();
    this->textures.clear();

    deallocate_mesh(this);
}

Model::Model(const std::string &path)
{
    Log::separator();
    Log::info("loading model: \"%s\"\n", path.c_str());

    std::string path_r = Utils::absolute_to_relative_path(path);
    std::string directory = path_r.substr(0, path_r.find_last_of('/'));
    std::string filename = path_r.substr(directory.length() + 1, path_r.length());

    this->directory = directory;
    this->filename = filename;
    
    std::string error = load_model(this);
    if (error != "")
    {
        directory = "";
        filename = "";

        Log::error("asset_load_model: \"%s\"\n", error.c_str());
        Log::separator();

        return;
    }

    Log::info("done!\n");
    Log::separator();
}

Model::~Model()
{
    for (auto& mesh : this->meshes)
        delete mesh;
    this->meshes.clear();

    for (auto& joint : this->joints)
        delete joint;
    this->joints.clear();
}

Animation::Animation(const std::string &path, Model *model)
{
    Log::separator();
    Log::info("loading animation: \"%s\"\n", path.c_str());

    std::string path_r = Utils::absolute_to_relative_path(path);
    std::string directory = path_r.substr(0, path_r.find_last_of('/'));
    std::string filename = path_r.substr(directory.length() + 1, path_r.length());

    this->directory = directory;
    this->filename = filename;
    
    std::string error = load_animation(this, model);
    if (error != "")
    {
        directory = "";
        filename = "";

        Log::error("asset_load_animation: \"%s\"\n", error.c_str());
        Log::separator();

        return;
    }

    Log::info("done!\n");
    Log::separator();
}

// TODO
Animation::~Animation()
{
    // ...
}

// ======================================================================================

std::string load_model(Model *model)
{
    std::string fullpath = model->directory+"/"+model->filename;
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(fullpath, aiProcess_Triangulate
                                             // // | aiProcess_MakeLeftHanded
                                             // // | aiProcess_PreTransformVertices
                                             | aiProcess_FlipUVs
                                             | aiProcess_JoinIdenticalVertices
                                             | aiProcess_CalcTangentSpace
                                             | aiProcess_GenSmoothNormals
                                             | aiProcess_FindInvalidData
                                             | aiProcess_ValidateDataStructure
                                             | 0);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        return importer.GetErrorString();

    process_node(model, scene, scene->mRootNode);

    return "";
}

void process_node(Model *model, const aiScene *scene, aiNode *node)
{
    for(uint32 i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];

        model->meshes.push_back(process_mesh(model, mesh, scene));
    }

    for(uint32 i = 0; i < node->mNumChildren; i++)
    {
        process_node(model, scene, node->mChildren[i]);
    }
}

Mesh* process_mesh(Model *model, aiMesh *mesh, const aiScene *scene)
{
    // Vertices.
    std::vector<Vertex> vertices;
    for (uint32 i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        for (uint32 i = 0; i < MAX_JOINT_INFLUENCE; i++)
        {
            vertex.jointIDs[i] = -1;
            vertex.weights[i] = 0.0f;            
        }
        
        glm::vec3 vector; 

        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z; 

        vertex.position = vector;

        vector.x = mesh->mNormals[i].x;
        vector.y = mesh->mNormals[i].y;
        vector.z = mesh->mNormals[i].z;

        vertex.normal = vector; 

        if (mesh->mTextureCoords[0])
        {
            glm::vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x; 
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.texCoords = vec;
        }
        else
            vertex.texCoords = glm::vec2(0.0f, 0.0f);

        vector.x = mesh->mTangents[i].x;
        vector.y = mesh->mTangents[i].y;
        vector.z = mesh->mTangents[i].z;

        vertex.tangent = vector;

        vector.x = mesh->mBitangents[i].x;
        vector.y = mesh->mBitangents[i].y;
        vector.z = mesh->mBitangents[i].z;

        vertex.bitangent = vector;
	
        vertices.push_back(vertex);
    }

    // Indices.
    std::vector<uint32> indices;
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }  

    // Materials / Textures.
    std::vector<Texture> textures;
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

    std::vector<Texture> diffuseMaps = load_material_textures(model->directory, material, aiTextureType_DIFFUSE, "texture_diffuse");
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
    std::vector<Texture> specularMaps = load_material_textures(model->directory, material, aiTextureType_SPECULAR, "texture_specular");
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    std::vector<Texture> normalMaps = load_material_textures(model->directory, material, aiTextureType_HEIGHT, "texture_normal");
    textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
    std::vector<Texture> heightMaps = load_material_textures(model->directory, material, aiTextureType_AMBIENT, "texture_height");
    textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

    // Joints
    process_joint_data(model, vertices, mesh);
    
    return new Mesh(vertices, indices, textures);
}

void process_joint_data(Model *model, std::vector<Vertex> &vertices, aiMesh* mesh)
{
    auto& jointTransforms = model->jointTransforms;
    uint32& jointCount = model->jointCount;

    for (uint32 i = 0; i < mesh->mNumBones; ++i)
    {
        int32 jointID = -1;
        std::string name = mesh->mBones[i]->mName.C_Str();

        // DEBUG
        //Log::debug("joint= \"%s\"\n", name.c_str());

        if (jointTransforms.find(name) == jointTransforms.end())
        {
            JointTransform jointTrans;
            jointTrans.ID = jointCount;
            jointTrans.localTransform = convert_aimatrix_to_glm(mesh->mBones[i]->mOffsetMatrix);

            jointTransforms[name] = jointTrans;

            jointID = jointCount;
            jointCount++;
        } else
        {
            jointID = jointTransforms[name].ID;
        }

        // DEBUG
        assert(jointID != -1);

        auto weights = mesh->mBones[i]->mWeights;
        uint32 numWeights = mesh->mBones[i]->mNumWeights;

        for (uint32 j = 0; j < numWeights; ++j)
        {
            uint32 vertexID = weights[j].mVertexId;
            float32 weight = weights[j].mWeight;

            // DEBUG
            assert(vertexID <= vertices.size());

            Vertex vertex = vertices[vertexID];
            for (uint32 k = 0; k < MAX_JOINT_INFLUENCE; ++k)
            {
                if (vertex.jointIDs[k] < 0)
                {
                    vertex.weights[k] = weight;
                    vertex.jointIDs[k] = jointID;
                    break;
                }
            }
        }
    }
}

std::vector<Texture> load_material_textures(const std::string &directory, aiMaterial *mat, aiTextureType type, std::string typeName)
{
    std::vector<Texture> textures;
    for(uint32 i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);

        // check if texture was loaded before
        bool skip = false;
        for(uint32 j = 0; j < textures.size(); j++)
        {
            if(std::strcmp(textures[j].filename.data(), str.C_Str()) == 0)
            {
                skip = true;
                break;
            }
        }

        std::string filename(str.C_Str());
        std::string fullpath = directory+"/"+filename;

        // if texture hasn't been loaded already, load it
        if(!skip)
        {
            Texture t;
            t.ID = load_texture_from_file(fullpath.c_str());
            t.type = typeName;
            t.filename = str.C_Str();
            textures.push_back(t);
        }
    }

    return textures;
}

uint32 load_texture_from_file(const std::string &path)
{
    Log::info("loading texture: \"%s\"\n", path.c_str());

    uint32 textureID;
    glGenTextures(1, &textureID); // TODO: OGL
    
    int width, height, nrComponents;
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        // TODO: OGL
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;
        else
            format = GL_RGBA;

        // TODO: OGL
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // TODO: OGL
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else
        Log::error("failed to load texture from: %s\n", path.c_str());

    stbi_image_free(data);

    return textureID;
}

std::string load_animation(Animation *animation, Model *model)
{
    std::string fullpath = animation->directory+"/"+animation->filename;
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(fullpath, aiProcess_Triangulate);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        return importer.GetErrorString();

    auto aiAnim = scene->mAnimations[0];
    animation->duration = aiAnim->mDuration;
    animation->ticksPerSecond = aiAnim->mTicksPerSecond;

    aiMatrix4x4 globalTransformation = scene->mRootNode->mTransformation;
    globalTransformation = globalTransformation.Inverse();

    process_animation_hierarchy(animation->rootNode, scene->mRootNode);

    setup_animation_joints(aiAnim, model);

    return "";
}

void process_animation_hierarchy(AnimationNode& dest, const aiNode* src)
{
    // DEBUG
    assert(src);

    dest.name = src->mName.data;
    dest.transformation = convert_aimatrix_to_glm(src->mTransformation);
    dest.childrenCount = src->mNumChildren;

    for (uint32 i = 0; i < src->mNumChildren; i++)
    {
        AnimationNode newNode;
        process_animation_hierarchy(newNode, src->mChildren[i]);
        dest.children.push_back(newNode);
    }
}

void setup_animation_joints(const aiAnimation* aiAnim, Model *model)
{
    uint32 size = aiAnim->mNumChannels;

    auto& jointTransforms = model->jointTransforms;
    uint32& jointCount = model->jointCount;

    for (uint32 i = 0; i < size; i++)
    {
        auto channel = aiAnim->mChannels[i];
        std::string jointName = channel->mNodeName.data;

        if (jointTransforms.find(jointName) == jointTransforms.end())
        {
            jointTransforms[jointName].ID = jointCount;
            jointCount++;
        }

        model->joints.push_back(create_animation_joint(channel->mNodeName.data, jointTransforms[channel->mNodeName.data].ID, channel));
    }

    //TODO: m_BoneInfoMap = jointTransforms;
}

Joint* create_animation_joint(const std::string& name, int32 ID, const aiNodeAnim* channel)
{
    Joint *joint = new Joint(ID, name);

    joint->numPositions = channel->mNumPositionKeys;
    for (uint32 i = 0; i < joint->numPositions; ++i)
    {
        aiVector3D aiPosition = channel->mPositionKeys[i].mValue;
        float32 timestamp = channel->mPositionKeys[i].mTime;

        KeyPosition data;
        data.position = convert_aivec_to_glm(aiPosition);
        data.timestamp = timestamp;

        joint->positions.push_back(data);
    }

    joint->numRotations = channel->mNumRotationKeys;
    for (uint32 i = 0; i < joint->numRotations; ++i)
    {
        aiQuaternion aiOrientation = channel->mRotationKeys[i].mValue;
        float32 timestamp = channel->mRotationKeys[i].mTime;

        KeyRotation data;
        data.orientation = convert_aiquat_to_glm(aiOrientation);
        data.timestamp = timestamp;

        joint->rotations.push_back(data);
    }

    joint->numScales = channel->mNumScalingKeys;
    for (uint32 i = 0; i < joint->numScales; ++i)
    {
        aiVector3D scale = channel->mScalingKeys[i].mValue;
        float32 timestamp = channel->mScalingKeys[i].mTime;

        KeyScale data;
        data.scale = convert_aivec_to_glm(scale);
        data.timestamp = timestamp;

        joint->scales.push_back(data);
    }

    // DEBUG
    Log::debug("TEST1=%d\n", joint->positions.size());
    Log::debug("TEST2=%d\n", joint->rotations.size());
    Log::debug("TEST3=%d\n", joint->scales.size());

}

glm::mat4 convert_aimatrix_to_glm(const aiMatrix4x4& from)
{
    glm::mat4 to;
    //the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
    to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
    to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
    to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
    to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
    return to;
}

glm::vec3 convert_aivec_to_glm(const aiVector3D& vec) 
{ 
    return glm::vec3(vec.x, vec.y, vec.z); 
}

glm::quat convert_aiquat_to_glm(const aiQuaternion& pOrientation)
{
    return glm::quat(pOrientation.w, pOrientation.x, pOrientation.y, pOrientation.z);
}

// ================================================================================

void allocate_mesh(Mesh *mesh)
{
    glGenVertexArrays(1, &mesh->VAO);
    glGenBuffers(1, &mesh->VBO);
    glGenBuffers(1, &mesh->IBO);
  
    glBindVertexArray(mesh->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO);

    if (!mesh->vertices.empty())
    {
        glBufferData(GL_ARRAY_BUFFER,
                     mesh->vertices.size() * sizeof(Vertex),
                     &mesh->vertices[0], GL_STATIC_DRAW);  

        // vertex positions
        glEnableVertexAttribArray(0);	
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                              (void*)0);

        // vertex normals
        glEnableVertexAttribArray(1);	
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                              (void*)offsetof(Vertex, normal));

        // vertex texture coords
        glEnableVertexAttribArray(2);	
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                              (void*)offsetof(Vertex, texCoords));

        // vertex tangent
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                              (void*)offsetof(Vertex, tangent));

        // vertex bitangent
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                              (void*)offsetof(Vertex, bitangent));

        // vertex jointIDs
        glEnableVertexAttribArray(5);
        glVertexAttribIPointer(5, MAX_JOINT_INFLUENCE, GL_INT, sizeof(Vertex), 
                               (void*)offsetof(Vertex, jointIDs));

        // vertex weights
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, MAX_JOINT_INFLUENCE, GL_FLOAT, GL_FALSE, sizeof(Vertex), 
                              (void*)offsetof(Vertex, weights));         
    }

    if (!mesh->indices.empty())
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->IBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     mesh->indices.size() * sizeof(uint32), 
                     &mesh->indices[0], GL_STATIC_DRAW);
    }
    
    glBindVertexArray(0);
}

void deallocate_mesh(Mesh *mesh)
{
    glDeleteVertexArrays(1, &mesh->VAO);
    glDeleteBuffers(1, &mesh->VBO);
    glDeleteBuffers(1, &mesh->IBO);
}
