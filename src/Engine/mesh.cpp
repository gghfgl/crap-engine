#include "mesh.h"

static void allocate_mesh(mesh_t *Mesh);
static void process_node(model_t *Model, aiNode *node, const aiScene *scene);
static mesh_t* process_mesh(model_t *Model, aiMesh *mesh, const aiScene *scene);
static std::vector<texture_t> load_material_textures(model_t *Model, aiMaterial *mat, aiTextureType type, std::string typeName);
static uint32 load_texture_from_file(const char *path, const std::string &directory);

mesh_t* AllocAndInit(std::vector<vertex_t> vertices,
                     std::vector<uint32> indices,
                     std::vector<texture_t> textures)
{
    mesh_t *Mesh = new mesh_t;
    Mesh->Vertices = vertices;
    Mesh->Indices = indices;
    Mesh->Textures = textures;

    allocate_mesh(Mesh);

    return Mesh;
}

void Delete(mesh_t *Mesh)
{
    glDeleteVertexArrays(1, &Mesh->VAO);
    glDeleteBuffers(1, &Mesh->VBO);
    glDeleteBuffers(1, &Mesh->IBO);

    Mesh->Vertices.clear();
    Mesh->Indices.clear();
    Mesh->Textures.clear();

    delete Mesh;
}

mesh_t* CreatePrimitiveSphereMesh(float32 margin, float32 radius, uint32 stacks, uint32 slices)
{
    uint32 nbVerticesPerSphere = 0;
    std::vector<vertex_t> vertices;
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
            vertex_t Vertex;
            Vertex.Position = glm::vec3(x * radius + margin, y * radius, z * radius);
            vertices.push_back(Vertex);
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

    std::vector<texture_t> tEmpty;
    mesh_t *Mesh = AllocAndInit(vertices, indices, tEmpty);

    return Mesh;
}

model_t* LoadModelFromFile(std::string const &path)
{
    model_t *Model = new model_t;
    Assimp::Importer importer;
    // const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate
    // 				       | aiProcess_FlipUVs
    // 				       | aiProcess_CalcTangentSpace
    // 				       | aiProcess_JoinIdenticalVertices);

    const aiScene *scene = importer.ReadFile(path, aiProcess_FlipWindingOrder
                                             //						 | aiProcess_MakeLeftHanded
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
        printf("ASSIMP::Error '%s'\n", importer.GetErrorString());
        return nullptr;
    }

    Model->Directory = path.substr(0, path.find_last_of('\\'));
    Model->ObjFilename = path.substr(Model->Directory.length() + 1, path.length());

    // DEBUG:
    printf("==============================\n");
    printf("load model from directory: %s\n", Model->Directory.c_str());
    printf("load model from file: %s\n", Model->ObjFilename.c_str());

    process_node(Model, scene->mRootNode, scene);

    return Model;
}

void Delete(model_t *Model)
{
    for (auto& m : Model->Meshes)
        Delete(m);
    Model->Meshes.clear();

    for (auto& m : Model->TexturesLoadedCache)
        glDeleteTextures(1, &m.Id);
    Model->TexturesLoadedCache.clear();
    delete Model;
}

static void allocate_mesh(mesh_t *Mesh)
{
    glGenVertexArrays(1, &Mesh->VAO);
    glGenBuffers(1, &Mesh->VBO);
    glGenBuffers(1, &Mesh->IBO);
  
    glBindVertexArray(Mesh->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, Mesh->VBO);

    if (!Mesh->Vertices.empty())
    {
        glBufferData(GL_ARRAY_BUFFER,
                     Mesh->Vertices.size() * sizeof(vertex_t),
                     &Mesh->Vertices[0], GL_STATIC_DRAW);  

        // vertex positions
        glEnableVertexAttribArray(0);	
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t),
                              (void*)0);

        // vertex normals
        glEnableVertexAttribArray(1);	
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t),
                              (void*)offsetof(vertex_t, Normal));

        // vertex texture coords
        glEnableVertexAttribArray(2);	
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t),
                              (void*)offsetof(vertex_t, TexCoords));

        // vertex tangent
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t),
                              (void*)offsetof(vertex_t, Tangent));

        // vertex bitangent
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t),
                              (void*)offsetof(vertex_t, Bitangent));
    }

    if (!Mesh->Indices.empty())
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Mesh->IBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     Mesh->Indices.size() * sizeof(uint32), 
                     &Mesh->Indices[0], GL_STATIC_DRAW);
    }
    
    glBindVertexArray(0);
}

static void process_node(model_t *Model, aiNode *node, const aiScene *scene)
{
    // process all the node's meshes (if any)
    for(uint32 i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]]; 
        Model->Meshes.push_back(process_mesh(Model, mesh, scene));
    }

    // then do the same for each of its children
    for(uint32 i = 0; i < node->mNumChildren; i++)
    {
        process_node(Model, node->mChildren[i], scene);
    }
}

static mesh_t* process_mesh(model_t *Model, aiMesh *mesh, const aiScene *scene)
{
    std::vector<vertex_t> vertices;
    std::vector<uint32> indices;
    std::vector<texture_t> textures;

    // Vertices
    for (uint32 i = 0; i < mesh->mNumVertices; i++)
    {
        vertex_t Vertex;
        glm::vec3 vector; 

        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z; 
        Vertex.Position = vector;

        vector.x = mesh->mNormals[i].x;
        vector.y = mesh->mNormals[i].y;
        vector.z = mesh->mNormals[i].z;
        Vertex.Normal = vector; 

        if(mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
        {
            glm::vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x; 
            vec.y = mesh->mTextureCoords[0][i].y;
            Vertex.TexCoords = vec;
        }
        else
            Vertex.TexCoords = glm::vec2(0.0f, 0.0f);

        vector.x = mesh->mTangents[i].x;
        vector.y = mesh->mTangents[i].y;
        vector.z = mesh->mTangents[i].z;
        Vertex.Tangent = vector;

        vector.x = mesh->mBitangents[i].x;
        vector.y = mesh->mBitangents[i].y;
        vector.z = mesh->mBitangents[i].z;
        Vertex.Bitangent = vector;
	
        vertices.push_back(Vertex);
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
    std::vector<texture_t> diffuseMaps = load_material_textures(Model,
                                                                material,
                                                                aiTextureType_DIFFUSE,
                                                                "texture_diffuse");
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

    // 2. specular maps
    std::vector<texture_t> specularMaps = load_material_textures(Model,
                                                                 material,
                                                                 aiTextureType_SPECULAR,
                                                                 "texture_specular");
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

    // 3. normal maps
    std::vector<texture_t> normalMaps = load_material_textures(Model,
                                                               material,
                                                               aiTextureType_HEIGHT,
                                                               "texture_normal");
    textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

    // 4. height maps
    std::vector<texture_t> heightMaps = load_material_textures(Model,
                                                               material,
                                                               aiTextureType_AMBIENT,
                                                               "texture_height");
    textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

    return AllocAndInit(vertices, indices, textures);
}

static std::vector<texture_t> load_material_textures(model_t *Model,
                                                     aiMaterial *mat,
                                                     aiTextureType type,
                                                     std::string typeName)
{
    std::vector<texture_t> textures;
    for(uint32 i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);

        // check if texture was loaded before
        bool skip = false;
        for(uint32 j = 0; j < Model->TexturesLoadedCache.size(); j++)
        {
            if(std::strcmp(Model->TexturesLoadedCache[j].Path.data(), str.C_Str()) == 0)
            {
                textures.push_back(Model->TexturesLoadedCache[j]);
                skip = true;
                break;
            }
        }

        // if texture hasn't been loaded already, load it
        if(!skip)
        {
            texture_t Texture;
            Texture.Id = load_texture_from_file(str.C_Str(), Model->Directory);
            Texture.Type = typeName;
            Texture.Path = str.C_Str();
            textures.push_back(Texture);
            Model->TexturesLoadedCache.push_back(Texture);  // caching texture
        }
    }

    return textures;
}

static uint32 load_texture_from_file(const char *path, const std::string &directory)
{
    // DEBUG
    printf("texture=%s\n", path);
    
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
            format = GL_RGBA; // TODO: ??

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