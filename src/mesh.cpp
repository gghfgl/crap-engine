#include "mesh.h"

static void allocate_mesh(mesh_t *Mesh);
static void process_node(model_t *Model, aiNode *node, const aiScene *scene);
static mesh_t* process_mesh(model_t *Model, aiMesh *mesh, const aiScene *scene);
static std::vector<texture_t> load_material_textures(model_t *Model, aiMaterial *mat, aiTextureType type, std::string typeName);
static uint32 load_texture_from_file(const char *path, const std::string &directory);
static uint32 load_cubemap_texture_from_file(std::vector<std::string> faces);

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

void Delete(entity_t* Entity)
{
    Delete(Entity->Model);
    Delete(Entity->PickingSphere);
    delete Entity;
}

// TODO: make binary version?
int32 SaveSceneInTextFormat(char *filepath, std::map<uint32, entity_t*> *Scene)
{
    FILE *file;
    errno_t err;
    err = fopen_s(&file, filepath, "w");
    if (err != 0 || file == NULL)
    {
        printf("open file error!\n");
        return err;
    }

    for (auto it = Scene->begin(); it != Scene->end(); it++)
    {
        fprintf(file, "###\n");
        fprintf(file, "directory=%s\n", it->second->Model->Directory.c_str());
        fprintf(file, "filename=%s\n", it->second->Model->ObjFilename.c_str());
        fprintf(file, "id=%d\n", it->first);
        fprintf(file, "posx=%f\n", it->second->Position.x);
        fprintf(file, "posy=%f\n", it->second->Position.y);
        fprintf(file, "posz=%f\n", it->second->Position.z);
        fprintf(file, "scale=%f\n", it->second->Scale);
        fprintf(file, "rotate=%f\n", it->second->Rotate);
    }

    fclose(file);

    // DEBUG:
    printf("saved: %s\n", filepath);
    return 0;
}

int32 OpenSceneFromTextFormat(char *filepath, std::map<uint32, entity_t*> *Scene)
{
    std::ifstream file(filepath);
    if (!file.is_open())
    {
        printf("open file error!\n");
        return -1;
    }

    for (auto it = Scene->begin(); it != Scene->end(); it++)
        Delete(it->second);
    Scene->clear();

    uint32 fetch = 0;
    uint32 nbFields = 8;
    uint32 id = 0;
    std::string fullpath = "";
    glm::vec3 position = glm::vec3(0.0f);
    float32 scale = 0.0f;
    float32 rotate = 0.0f;
    std::string line;
    while (std::getline(file, line))
    {
        line.erase(std::remove_if(line.begin(), line.end(), isspace),
                   line.end());

        if(line[0] == '#' || line.empty())
            continue;

        auto delimiterPos = line.find("=");
        auto key = line.substr(0, delimiterPos);
        auto value = line.substr(delimiterPos + 1);

        if (key.compare("directory") == 0)
        {
            fullpath = value ;
            fetch++;
        }

        if (key.compare("filename") == 0)
        {
            fullpath += "\\" + value;
            fetch++;
        }
	    
        if (key.compare("id") == 0)
        {
            id = std::stoi(value);
            fetch++;
        }

        if (key.compare("posx") == 0)
        {
            position.x = std::stof(value);
            fetch++;
        }

        if (key.compare("posy") == 0)
        {
            position.y = std::stof(value);
            fetch++;
        }

        if (key.compare("posz") == 0)
        {
            position.z = std::stof(value);
            fetch++;
        }

        if (key.compare("scale") == 0)
        {
            scale = std::stof(value);
            fetch++;
        }

        if (key.compare("rotate") == 0)
        {
            rotate = std::stof(value);
            fetch++;
        }

        if (fetch == nbFields)
        {
            model_t *loadedModel = LoadModelFromFile(fullpath);
            entity_t *entity = new entity_t;
            entity->Model = loadedModel;
            entity->PickingSphere =  CreatePrimitiveSphereMesh(0.0f, 0.2f, 15, 15);
            entity->Position = position;
            entity->Scale = scale;
            entity->Rotate = rotate;

            Scene->insert({id, entity});

            printf("fullpath= %s\n", fullpath.c_str());
            printf("id= %d\n", id);
            printf("posx= %f\n", position.x);
            printf("posy= %f\n", position.y);
            printf("posz= %f\n", position.z);
            printf("scale= %f\n", scale);
            printf("rotate= %f\n", rotate);
		
            fetch = 0;
        }
    }

    file.close();

    // DEBUG:
    printf("opened: %s", filepath);
    return 0;
}

skybox_t* GenerateSkyboxFromFiles(std::vector<std::string> faces)
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

    uint32 textureID = load_cubemap_texture_from_file(faces);  

    skybox_t *Skybox = new skybox_t;
    Skybox->VAO = VAO;
    Skybox->VBO = VBO;
    Skybox->TextureId = textureID;
	
    return Skybox;
}

void Delete(skybox_t *Skybox)
{
    glDeleteVertexArrays(1, &Skybox->VAO);
    glDeleteBuffers(1, &Skybox->VBO);
    glDeleteTextures(1, &Skybox->TextureId);

    delete Skybox;
}

static uint32 load_cubemap_texture_from_file(std::vector<std::string> faces)
{
    uint32 textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

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

    return textureID;
}
