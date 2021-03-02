#pragma once

#define MAX_BONE_INFLUENCE 4

// Vertex represents a single point of a mesh.
// @textCoords: UV 2D texture coordinates.
// @jointIDs: ID of each joint that affects the vertex.
// @weights: how much each joint (from ids) affects the vertex. Must have a weight
// value by joint.
struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
    glm::vec3 tangent;
    glm::vec3 bitangent;
    uint32    jointIDs[MAX_BONE_INFLUENCE];
    float32   weights[MAX_BONE_INFLUENCE];
};

// Texture hold the data needed to draw textures.
struct Texture
{
    uint32 ID;
    std::string type;
    std::string path; // TODO: full? relative? absolute?
};

// Mesh hold the data needed to draw a model or a part of a model.
// @VAO: vertex array.
// @VBO: vertex buffer.
// @IBO: index buffer.
// @vertices: all vertices needed to make a mesh.
// @indices: all indicies to optimize vertices layout.
struct Mesh {
    Mesh();
    ~Mesh();

    uint32 VAO;
    uint32 VBO;
    uint32 IBO;
    std::vector<Vertex> vertices;
    std::vector<uint32> indices;
    std::vector<Texture> textures;
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
struct Joint {
    Joint(uint32 ID, std::string name, glm::mat4 localBindTransform);
    ~Joint();
    
    uint32 ID;
    std::string name;
    glm::mat4 animatedTransform;
    glm::mat4 localBindTransform;
    glm::mat4 inverseBindTransform;
    std::vector<Joint> children;

    void calc_inverse_bind_transform();
};

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
    std::vector<JointTransform> jointTransforms;
    float32 timestamp;
};

// TODO: useless??
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

// ASSET ACL
static std::string ASSET_load_meshes(const std::string &path, std::vector<*Mesh> &meshes);
static void ASSET_process_node(const aiScene *scene, std::vector<*Mesh> &meshes);
static Mesh* ASSET_process_mesh(aiMesh *mesh, const aiScene *scene);
static std::vector<Texture> ASSET_load_material_textures(aiMaterial *mat, aiTextureType type, std::string typeName);
static uint32 ASSET_load_texture_from_file(const std::string &path);

// Model represents the minimum stuff for loading 3D model.
// @meshes: the "skin" of the model.
struct Model {
    Model(const std::string &path);
    ~Model();
    
    std::string filename;
    std::string directory;
    std::vector<Mesh*> meshes;
};

// ====================================================================

// ASSET LOAD MESHES.
static std::string ASSET_load_meshes(const std::string &path, std::vector<*Mesh> &meshes)
{
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate
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

    ASSET_process_node(scene->mRootNode, scene);

    return;
    // TODO: ?
    // return importer.GetErrorString();
}

// ASSET PROCESS NODE.
static void ASSET_process_node(const aiScene *scene, &std::vector<*Mesh> meshes)
{
    aiNode *node = scene->mRootNode;
    for(uint32 i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]]; 
        meshes.push_back(ASSET_process_mesh(mesh, scene));
    }

    for(uint32 i = 0; i < node->mNumChildren; i++)
    {
        ASSET_process_node(node->mChildren[i], scene);
    }
}

// ASSET PROCESS MESH.
static Mesh* ASSET_process_mesh(aiMesh *mesh, const aiScene *scene)
{
    std::vector<Vertex> vertices;
    std::vector<uint32> indices;
    std::vector<Texture> textures;

    // Vertices.
    for (uint32 i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        glm::vec3 vector; 

        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z; 

        vertex.position = vector;

        vector.x = mesh->mNormals[i].x;
        vector.y = mesh->mNormals[i].y;
        vector.z = mesh->mNormals[i].z;

        vertex.normal = vector; 

        // does the mesh contain texture coordinates?
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
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }  

    // Materials / Textures.
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

    // Diffuse maps.
    std::vector<Texture> diffuseMaps = ASSET_load_material_textures(material, aiTextureType_DIFFUSE, "texture_diffuse");
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

    // Specular maps.
    std::vector<Texture> specularMaps = ASSET_load_material_textures(material, aiTextureType_SPECULAR, "texture_specular");
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

    // Normal maps.
    std::vector<Texture> normalMaps = ASSET_load_material_textures(material, aiTextureType_HEIGHT, "texture_normal");
    textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

    // Height maps.
    std::vector<Texture> heightMaps = ASSET_load_material_textures(material, aiTextureType_AMBIENT, "texture_height");
    textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

    return new Mesh(vertices, indices, textures);
}

// ASSET LOAD MATERIAL TEXTURES
static std::vector<Texture> ASSET_load_material_textures(aiMaterial *mat, aiTextureType type, std::string typeName)
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
            if(std::strcmp(textures[j].path.data(), str.C_Str()) == 0)
            {
                skip = true;
                break;
            }
        }

        // if texture hasn't been loaded already, load it
        if(!skip)
        {
            Texture t;
            t.ID = ASSET_load_texture_from_file(str.C_Str()); // TODO
            t.type = typeName;
            t.path = str.C_Str();
            textures.push_back(t);
        }
    }

    return textures;
}

// ASSET LOAD TEXTURE FROM FILE
static uint32 ASSET_load_texture_from_file(const std::string &path)
{
    Log::info("loading texture [file]: %s\n", path.c_str());

    uint32 textureID;
    glGenTextures(1, &textureID); // TODO: OGL

    int width, height, nrComponents;
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
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
        Log::warn("failed to load texture from: %s\n", path.c_str());

    stbi_image_free(data);

    return textureID;
}
