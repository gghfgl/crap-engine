#include "model.h"

Mesh::Mesh(std::vector<Vertex> vertices,
           std::vector<uint32> indices,
           std::vector<Texture> textures)
{
    this->Vertices = vertices;
    this->Indices = indices;
    this->Textures = textures;

    this->allocate_mesh();
}

// primtive sphere constructor
// Mesh::Mesh(float32 margin, float32 radius, uint32 stacks, uint32 slices)
// {
//     uint32 nbVerticesPerSphere = 0;
//     std::vector<Vertex> vertices;
//     std::vector<uint32> indices;

//     for (uint32 i = 0; i <= stacks; i++)
//     {
//         GLfloat V   = i / (float) stacks;
//         GLfloat phi = V * glm::pi <float> ();
        
//         for (uint32 j = 0; j <= slices; ++j)
//         {
//             GLfloat U = j / (float) slices;
//             GLfloat theta = U * (glm::pi <float> () * 2);
            
//             // Calc The Vertex Positions
//             GLfloat x = cosf (theta) * sinf (phi);
//             GLfloat y = cosf (phi);
//             GLfloat z = sinf (theta) * sinf (phi);
//             Vertex vertex;
//             vertex.position = glm::vec3(x * radius + margin, y * radius, z * radius);
//             vertices.push_back(vertex);
//             nbVerticesPerSphere += 1; // nb vertices per sphere reference
//         }
//     }

//     for (uint32 i = 0; i < slices * stacks + slices; ++i)
//     {        
//         indices.push_back (i);
//         indices.push_back (i + slices + 1);
//         indices.push_back (i + slices);
        
//         indices.push_back (i + slices + 1);
//         indices.push_back (i);
//         indices.push_back (i + 1);
//     }

//     std::vector<Texture> tEmpty;

//     this->Vertices = vertices;
//     this->Indices = indices;
//     this->Textures = tEmpty;

//     this->allocate_mesh();
// }

Mesh::~Mesh()
{
    glDeleteVertexArrays(1, &this->VAO);
    glDeleteBuffers(1, &this->VBO);
    glDeleteBuffers(1, &this->IBO);

    this->Vertices.clear();
    this->Indices.clear();
    this->Textures.clear();
}

void Mesh::allocate_mesh()
{
    glGenVertexArrays(1, &this->VAO);
    glGenBuffers(1, &this->VBO);
    glGenBuffers(1, &this->IBO);
  
    glBindVertexArray(this->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, this->VBO);

    if (!this->Vertices.empty())
    {
        glBufferData(GL_ARRAY_BUFFER,
                     this->Vertices.size() * sizeof(Vertex),
                     &this->Vertices[0], GL_STATIC_DRAW);  

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
    }

    if (!this->Indices.empty())
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->IBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     this->Indices.size() * sizeof(uint32), 
                     &this->Indices[0], GL_STATIC_DRAW);
    }
    
    glBindVertexArray(0);
}

// ======================================

Model::Model(std::string const &path)
{
    Log::info("=== BEGIN: New Model\n");

    // convert to relative path
    auto p = std::filesystem::proximate(path);
    std::string p_string{p.u8string()};

    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(p_string, aiProcess_FlipWindingOrder
                                             // | aiProcess_MakeLeftHanded
                                             | aiProcess_Triangulate
                                             | aiProcess_FlipUVs
                                             | aiProcess_PreTransformVertices
                                             | aiProcess_JoinIdenticalVertices
                                             | aiProcess_CalcTangentSpace
                                             | aiProcess_GenSmoothNormals
                                             | aiProcess_FixInfacingNormals
                                             | aiProcess_FindInvalidData
                                             | aiProcess_ValidateDataStructure
                                             | 0);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        this->directory = "";
        this->objFilename = "";

        Log::error("\tASSIMP: '%s'\n", importer.GetErrorString());
        Log::info("=== END: New Model\n");

        return; // TODO: error management
    }

    this->directory = p_string.substr(0, p_string.find_last_of('/'));
    this->objFilename = p_string.substr(this->directory.length() + 1, p_string.length());

    Log::info("\tload model from directory: %s\n", this->directory.c_str());
    Log::info("\tload model from file: %s\n", this->objFilename.c_str());

    this->process_node(scene->mRootNode, scene);
    // TODO: rework this crap
    this->boundingBox = new BoundingBox(this->maxComponents.x, this->maxComponents.y, this->maxComponents.z);
    Log::info("=== END: New Model\n");
}

Model::~Model()
{
    for (auto& m : this->Meshes)
        delete m;
    this->Meshes.clear();

    for (auto& m : this->TexturesLoadedCache)
        glDeleteTextures(1, &m.ID);
    this->TexturesLoadedCache.clear();
}

void Model::process_node(aiNode *node, const aiScene *scene)
{
    // process all the node's meshes (if any)
    for(uint32 i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]]; 
        this->Meshes.push_back(this->process_mesh(mesh, scene));
    }

    // then do the same for each of its children
    for(uint32 i = 0; i < node->mNumChildren; i++)
    {
        this->process_node(node->mChildren[i], scene);
    }
}

Mesh* Model::process_mesh(aiMesh *mesh, const aiScene *scene)
{
    std::vector<Vertex> vertices;
    std::vector<uint32> indices;
    std::vector<Texture> textures;

    // Vertices
    for (uint32 i = 0; i < mesh->mNumVertices; i++)
    {
        // TODO: rework this crap
        // Tracking max values for bounding box
        if (mesh->mVertices[i].x > this->maxComponents.x)
            this->maxComponents.x = mesh->mVertices[i].x;
        if (mesh->mVertices[i].y > this->maxComponents.y)
            this->maxComponents.y = mesh->mVertices[i].y;
        if (mesh->mVertices[i].z > this->maxComponents.z)
            this->maxComponents.z = mesh->mVertices[i].z;

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
        if(mesh->mTextureCoords[0])
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

    // Indices
    for(unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for(unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }  

    // Materials / Textures
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
    // 1. diffuse maps
    std::vector<Texture> diffuseMaps = this->load_material_textures(material,
                                                                      aiTextureType_DIFFUSE,
                                                                      "texture_diffuse");
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

    // 2. specular maps
    std::vector<Texture> specularMaps = this->load_material_textures(material,
                                                                       aiTextureType_SPECULAR,
                                                                       "texture_specular");
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

    // 3. normal maps
    std::vector<Texture> normalMaps = this->load_material_textures(material,
                                                                     aiTextureType_HEIGHT,
                                                                     "texture_normal");
    textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

    // 4. height maps
    std::vector<Texture> heightMaps = this->load_material_textures(material,
                                                                     aiTextureType_AMBIENT,
                                                                     "texture_height");
    textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

    return new Mesh(vertices, indices, textures);
}

std::vector<Texture> Model::load_material_textures(aiMaterial *mat,
                                                   aiTextureType type,
                                                   std::string typeName)
{
    std::vector<Texture> Textures;
    for(uint32 i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);

        // check if texture was loaded before
        bool skip = false;
        for(uint32 j = 0; j < this->TexturesLoadedCache.size(); j++)
        {
            if(std::strcmp(this->TexturesLoadedCache[j].path.data(), str.C_Str()) == 0)
            {
                Textures.push_back(this->TexturesLoadedCache[j]);
                skip = true;
                break;
            }
        }

        // if texture hasn't been loaded already, load it
        if(!skip)
        {
            Texture texture;
            texture.ID = this->load_texture_from_file(str.C_Str(), this->directory);
            texture.type = typeName;
            texture.path = str.C_Str();
            Textures.push_back(texture);
            this->TexturesLoadedCache.push_back(texture);  // caching texture
        }
    }

    return Textures;
}

uint32 Model::load_texture_from_file(const char *path, const std::string &directory)
{
    Log::info("\ttexture=%s\n", path);
    
    std::string filename = std::string(path);
    filename = directory + '/' + filename;

    uint32 textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;
        else
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else
        Log::warn("\tTexture failed to load at path: '%s'\n", path);

    stbi_image_free(data);

    return textureID;
}

BoundingBox::BoundingBox(float32 maxX, float32 maxY, float32 maxZ)
{
    float32 scaleX = maxX * 1.3;
    float32 scaleY = maxY * 1.3;
    float32 scaleZ = maxZ * 1.3;

    float32 bbVertices[] = {
        -scaleX,  scaleY, -scaleZ,
        -scaleX, -scaleY, -scaleZ,
        scaleX, -scaleY, -scaleZ,
        scaleX, -scaleY, -scaleZ,
        scaleX,  scaleY, -scaleZ,
        -scaleX,  scaleY, -scaleZ,

        -scaleX, -scaleY,  scaleZ,
        -scaleX, -scaleY, -scaleZ,
        -scaleX,  scaleY, -scaleZ,
        -scaleX,  scaleY, -scaleZ,
        -scaleX,  scaleY,  scaleZ,
        -scaleX, -scaleY,  scaleZ,

        scaleX, -scaleY, -scaleZ,
        scaleX, -scaleY,  scaleZ,
        scaleX,  scaleY,  scaleZ,
        scaleX,  scaleY,  scaleZ,
        scaleX,  scaleY, -scaleZ,
        scaleX, -scaleY, -scaleZ,

        -scaleX, -scaleY,  scaleZ,
        -scaleX,  scaleY,  scaleZ,
        scaleX,  scaleY,  scaleZ,
        scaleX,  scaleY,  scaleZ,
        scaleX, -scaleY,  scaleZ,
        -scaleX, -scaleY,  scaleZ,

        -scaleX,  scaleY, -scaleZ,
        scaleX,  scaleY, -scaleZ,
        scaleX,  scaleY,  scaleZ,
        scaleX,  scaleY,  scaleZ,
        -scaleX,  scaleY,  scaleZ,
        -scaleX,  scaleY, -scaleZ,

        -scaleX, -scaleY, -scaleZ,
        -scaleX, -scaleY,  scaleZ,
        scaleX, -scaleY, -scaleZ,
        scaleX, -scaleY, -scaleZ,
        -scaleX, -scaleY,  scaleZ,
        scaleX, -scaleY,  scaleZ
    };

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(bbVertices), &bbVertices, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    this->VAO = VAO;
    this->VBO = VBO;
}
