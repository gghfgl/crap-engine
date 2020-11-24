#include "entity.h"

Mesh::Mesh(std::vector<Vertex> vertices,
           std::vector<uint32> indices,
           std::vector<Texture> textures)
{
    this->Vertices = vertices;
    this->Indices = indices;
    this->Textures = textures;

    this->allocate_mesh();
}

// primtive sphere cosntructor
Mesh::Mesh(float32 margin, float32 radius, uint32 stacks, uint32 slices)
{
    uint32 nbVerticesPerSphere = 0;
    std::vector<Vertex> vertices;
    std::vector<uint32> indices;

    for (uint32 i = 0; i <= stacks; i++)
    {
        GLfloat V   = i / (float) stacks;
        GLfloat phi = V * glm::pi <float> ();
        
        for (uint32 j = 0; j <= slices; ++j)
        {
            GLfloat U = j / (float) slices;
            GLfloat theta = U * (glm::pi <float> () * 2);
            
            // Calc The Vertex Positions
            GLfloat x = cosf (theta) * sinf (phi);
            GLfloat y = cosf (phi);
            GLfloat z = sinf (theta) * sinf (phi);
            Vertex vertex;
            vertex.position = glm::vec3(x * radius + margin, y * radius, z * radius);
            vertices.push_back(vertex);
            nbVerticesPerSphere += 1; // nb vertices per sphere reference
        }
    }

    for (uint32 i = 0; i < slices * stacks + slices; ++i)
    {        
        indices.push_back (i);
        indices.push_back (i + slices + 1);
        indices.push_back (i + slices);
        
        indices.push_back (i + slices + 1);
        indices.push_back (i);
        indices.push_back (i + 1);
    }

    std::vector<Texture> tEmpty;

    this->Vertices = vertices;
    this->Indices = indices;
    this->Textures = tEmpty;

    this->allocate_mesh();
}

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
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(path, aiProcess_FlipWindingOrder
                                             // | aiProcess_MakeLeftHanded
                                             | aiProcess_Triangulate
                                             | aiProcess_FlipUVs
                                             | aiProcess_PreTransformVertices
                                             | aiProcess_JoinIdenticalVertices
                                             | aiProcess_CalcTangentSpace
                                             | aiProcess_GenSmoothNormals
                                             | aiProcess_Triangulate
                                             | aiProcess_FixInfacingNormals
                                             | aiProcess_FindInvalidData
                                             | aiProcess_ValidateDataStructure
                                             | 0);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        printf("crap-engine: ASSIMP::Error '%s'\n", importer.GetErrorString());
    }

    this->directory = path.substr(0, path.find_last_of('/'));
    this->objFilename = path.substr(this->directory.length() + 1, path.length());

    // DEBUG:
    printf("==============================\n");
    printf("crap-engine: load model from directory: %s\n", this->directory.c_str());
    printf("crap-engine: load model from file: %s\n", this->objFilename.c_str());

    this->process_node(scene->mRootNode, scene);
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
    // DEBUG
    printf("crap-engine: texture=%s\n", path);
    
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
        printf("Texture failed to load at path: '%s'\n", path);

    stbi_image_free(data);

    return textureID;
}

// static uint32 LoadTextureFromFile(const std::string& path)
// {
//     int w, h, bits;
//     stbi_set_flip_vertically_on_load(1);
//     unsigned char *data = stbi_load(path.c_str(), &w, &h, &bits, STBI_rgb);

//     uint32 textureID;
//     glCreateTextures(GL_TEXTURE_2D, 1, &textureID);
//     glBindTexture(GL_TEXTURE_2D, textureID);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // TODO REPEAT ?
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // TODO REPEAT ?
//     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

//     stbi_image_free(data);

//     return textureID;
// }

// ======================================

Entity::~Entity()
{
    delete this->model;
    if (this->pickingSphere != nullptr)
        delete this->pickingSphere;
}

// ======================================

Terrain::Terrain(uint32 resolution, glm::vec3 unitSize, std::string const &modelFilePath)
{
    this->entity = new Entity;
    this->entity->pickingSphere = nullptr;
    this->entity->model = new Model(modelFilePath);

    this->resolution = resolution;
    this->unitSize = unitSize;
    this->instanceBufferID = 0;
    this->isGenerated = false;

    this->updateModelMatrices(resolution);
}

Terrain::~Terrain()
{
    delete this->entity;
    this->clearInstance();
}

void Terrain::updateModelMatrices(uint32 sideLenght)
{
    this->resolution = sideLenght;
    this->modelMatrices = new glm::mat4[sideLenght * sideLenght];    

    glm::vec3 size = { 1.0f, 1.0f, 1.0f };
    float32 posX = -((float32)sideLenght / 2.0f - 0.5f);

    uint32 index = 0;
    for (uint32 i = 0; i < sideLenght; i++)
    {
        float32 posZ = ((float32)sideLenght / 2.0f - 0.5f);
        for (uint32 y = 0; y < sideLenght; y++)
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, { posX, 0.0f, posZ });
            model = glm::scale(model, glm::vec3(1.0f));
            model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            this->modelMatrices[index] = model;

            posZ -= size.z;
            index++;
        }
        posX += size.x;
    }
}

void Terrain::clearInstance()
{
    delete[] this->modelMatrices;
    glDeleteBuffers(1, &this->instanceBufferID);
}

// ======================================

Skybox::Skybox(std::vector<std::string> faces)
{
    float skyboxVertices[] = {
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f,  1.0f
    };

    uint32 VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    this->VAO = VAO;
    this->VBO = VBO;
    this->load_cubemap_texture_from_file(faces);
}

Skybox::~Skybox()
{
    glDeleteVertexArrays(1, &this->VAO);
    glDeleteBuffers(1, &this->VBO);
    glDeleteTextures(1, &this->textureID);
}

void Skybox::load_cubemap_texture_from_file(std::vector<std::string> faces)
{
    glGenTextures(1, &this->textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, this->textureID);

    int width, height, nrChannels;
    for (uint32 i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        else
            printf("Cubemap texture failed to load at path: %s", faces[i].c_str());

        stbi_image_free(data);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}
