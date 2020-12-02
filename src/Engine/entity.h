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

struct Ground
{
    Ground(const char* name, std::string const &modelFilePath, uint32 resolution);
    ~Ground();
    void updateModelMatrices();
    void clearInstance();
    bool diffResolutionBuffer();
    
    Entity *entity;
    uint32 resolution; // default size
    int32 resolutionBuffer;
    uint32 instanceBufferID;
    bool isGenerated = false;
    glm::mat4 *modelMatrices;
    const char* name;
};

struct Skybox
{
    Skybox(const char* name, std::vector<std::string> faces);
    ~Skybox();

    uint32 VAO;
    uint32 VBO;
    uint32 textureID;
    const char* name;
    std::string cubeMapFilename;
    std::string directory;

private:
    void load_cubemap_texture_from_file(std::vector<std::string> faces);
};

// =====================================================

inline int32 SaveEntityListInTextFormat(const char *filepath, std::map<uint32, Entity*> *List)
{
    // DEBUG:
    printf("\n=== BEGIN: SaveEntityListInTextFormat\n");

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
    printf("=== END: SaveEntityListInTextFormat\n");

    return 0;
}

inline int32 SaveGroundListInTextFormat(const char *filepath, std::map<uint32, Ground*> *List)
{
    // DEBUG:
    printf("\n=== BEGIN: SaveGroundListInTextFormat\n");

    std::ofstream file;
    file.open(filepath);
    file << "HEADER_GROUND_LIST\n";
    for (auto it = List->begin(); it != List->end(); it++)
    {
        std::string objf = "";
        std::string dir = "";
        if (it->second->entity->model != nullptr)
        {
            dir = it->second->entity->model->directory.c_str();
            objf = it->second->entity->model->objFilename.c_str();
        }

        file << "###\n";
        file << "id=" << it->first << "\n";
        file << "name=" << it->second->name << "\n";
        file << "directory=" << dir << "\n";
        file << "objfile=" << objf << "\n";
        file << "resolution=" << it->second->resolution << "\n";
    }

    file.close();

    // DEBUG:
    printf("saved: %s\n", filepath);
    printf("=== END: SaveGroundListInTextFormat\n");

    return 0;
}

inline int32 OpenEntityListFromFileTextFormat(const char *filepath,
                                              std::map<uint32,Entity*> *List)
{
    // TODO: check header!
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

inline int32 OpenGroundListFromFile(const char *filepath,
                                    std::map<uint32,Ground*> *List)
{
    // DEBUG:
    printf("\n=== BEGIN: OpenGroundListFromFile\n");

    std::ifstream file(filepath);
    if (!file.is_open())
    {
        // DEBUG:
        printf("open file error!\n");
        return -1;
    }

    // Clear map
    for (auto it = List->begin(); it != List->end(); it++)
        delete it->second;
    List->clear();


    uint32 id = 1;
    std::string name = "unknown";
    std::string fullpath = "";
    uint32 resolution = 0;

    uint32 fetch = 0;
    uint32 nbFields = 5;
    std::string line;

    std::getline(file, line);
    if (line != "HEADER_GROUND_LIST")
    {
        // DEBUG:
        printf("\nfaile to read %s: header is missing", filepath);
        printf("=== END: OpenGroundListFromFile\n");
        return -1;
    }

    while (std::getline(file, line))
    {
        line.erase(std::remove_if(line.begin(), line.end(), isspace),
                   line.end());

        {
            if(line[0] == '#' || line.empty())
                continue;

            auto delimiterPos = line.find("=");
            auto key = line.substr(0, delimiterPos);
            auto value = line.substr(delimiterPos + 1);

            if (key.compare("id") == 0)
            {
                id = std::stoi(value);
                fetch++;
            }

            if (key.compare("name") == 0)
            {
                name = value;
                fetch++;
            }

            if (key.compare("directory") == 0)
            {
                fullpath = value ;
                fetch++;
            }

            if (key.compare("objfile") == 0)
            {
                    fullpath += "/" + value;
                    fetch++;
            }

            if (key.compare("resolution") == 0)
            {
                resolution = std::stoi(value);
                fetch++;
            }

            if (fetch == nbFields)
            {
                // in case of no objfile
                if (fullpath.length() == 1)
                    fullpath = "";

                // DEBUG:
                printf("fetching fields [%d]/[%d]\n", fetch, nbFields);
                /* printf("id= %d\n", id); */
                /* printf("name= %s\n", name.c_str()); */
                /* printf("fullpath= %s\n", fullpath.c_str()); */
                /* printf("resolution= %d\n", resolution); */

                // TODO
                char *nameCopy = new char[32];
                strncpy(nameCopy, name.c_str(), 32);
                nameCopy[32 - 1] = '\0';

                Ground *ground = new Ground(nameCopy, fullpath, resolution);
                List->insert({id, ground});
		
                fetch = 0;
            }            
        }
    }

    file.close();

    // DEBUG:
    printf("opened: %s\n", filepath);
    printf("=== END: OpenGroundListFromFile\n");

    return 0;
}
