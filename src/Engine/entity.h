#pragma once

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
    glm::vec3 tangent;
    glm::vec3 bitangent;
};

struct Texture
{
    uint32 ID;
    std::string type;
    std::string path;
};

struct Mesh
{
    Mesh(std::vector<Vertex> vertices,
         std::vector<uint32> indices,
         std::vector<Texture> textures);
    Mesh(float32 margin, float32 radius, uint32 stacks, uint32 slices);
    ~Mesh();
    
    uint32 VAO;
    uint32 VBO;
    uint32 IBO;

    std::vector<Vertex> Vertices;
    std::vector<uint32> Indices;
    std::vector<Texture> Textures;

private:
    void allocate_mesh();
};

struct Model
{
    Model(std::string const &path);
    ~Model();

    std::vector<Mesh*> Meshes;
    std::vector<Texture> TexturesLoadedCache;
    std::string objFilename;
    std::string directory;
    bool gammaCorrection;

private:
    void process_node(aiNode *node, const aiScene *scene);
    Mesh* process_mesh(aiMesh *mesh, const aiScene *scene);
    std::vector<Texture> load_material_textures(aiMaterial *mat,
                                                aiTextureType type,
                                                std::string typeName);
    uint32 load_texture_from_file(const char *path, const std::string &directory);
};

struct Entity
{
    ~Entity();
    
    Model *model;
    Mesh *pickingSphere;
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
    float32 scale = 1.0f;
    float32 rotate = 0.0f; // degres
};

// ======================================

struct Terrain
{
    Terrain(uint32 resolution, glm::vec3 unitSize, std::string const &modelFilePath);
    ~Terrain();
    void updateModelMatrices(uint32 sideLenght);
    void clearInstance();
    
    Entity *entity;
    uint32 resolution; // default size
    glm::vec3 unitSize;
    uint32 instanceBufferID;
    bool isGenerated = false;
    glm::mat4 *modelMatrices;
};

struct Skybox
{
    Skybox(std::vector<std::string> faces);
    ~Skybox();

    uint32 VAO;
    uint32 VBO;
    uint32 textureID;

private:
    void load_cubemap_texture_from_file(std::vector<std::string> faces);
};

// =====================================================

inline int32 SaveEntityListInTextFormat(const char *filepath, std::map<uint32, Entity*> *List)
{
    std::ofstream file;
    file.open(filepath);
    for (auto it = List->begin(); it != List->end(); it++)
    {
        file << "###\n";
        file << "directory=" << it->second->model->directory.c_str() << "\n";
        file << "filename=" << it->second->model->objFilename.c_str() << "\n";
        file << "id=" << it->first << "\n";
        file << "posx=" << it->second->position.x << "\n";
        file << "posy=" << it->second->position.y << "\n";
        file << "posz=" << it->second->position.z << "\n";
        file << "scale=" << it->second->scale << "\n";
        file << "rotate=" << it->second->rotate << "\n";
    }

    file.close();

    // DEBUG:
    printf("saved: %s\n", filepath);
    return 0;
}


inline int32 OpenEntityListFromFileTextFormat(const char *filepath,
                                              std::map<uint32,Entity*> *List)
{
    std::ifstream file(filepath);
    if (!file.is_open())
    {
        printf("open file error!\n");
        return -1;
    }

    for (auto it = List->begin(); it != List->end(); it++)
        delete it->second;
    List->clear();

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
            Entity *entity = new Entity();
            entity->model = new Model(fullpath);
            entity->pickingSphere =  new Mesh(0.0f, 0.2f, 15, 15);
            entity->position = position;
            entity->scale = scale;
            entity->rotate = rotate;

            List->insert({id, entity});

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
