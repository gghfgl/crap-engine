#pragma once

enum EntityDirection
{
    ENTITY_FORWARD,
    ENTITY_BACKWARD,
    ENTITY_LEFT,
    ENTITY_RIGHT,
    ENTITY_UP,
    ENTITY_DOWN
};

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
    int scale = 1;
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
    glm::mat4 *modelMatrices;
    const char* name;
};

struct Module
{
    Module(const char* name, std::string const &modelFilePath);
    ~Module();
    
    Entity *entity;
    const char* name;
};

struct Skybox
{
    Skybox(const char* name, std::string directoryPath);
    ~Skybox();
    void loadCubeMapTextureFromFile(std::string directoryPath);

    uint32 VAO;
    uint32 VBO;
    uint32 textureID;
    const char* name;
    std::string directory;
};

struct Player
{
    Player(const char* name, std::string const &modelFilePath, glm::vec3 position);
    ~Player();
    void UpdatePositionFromDirection(EntityDirection direction, float32 deltaTime, float32 acceleration);
    
    Entity *entity;
    const char* name;

    float32 m_speed = 30.0f;
};

// =====================================================

inline int32 SaveModuleListInTextFormat(const char *filepath, std::map<uint32, Module*> *List)
{
    Log::info("=== BEGIN: SaveModuleListInTextFormat\n");

    std::ofstream file;
    file.open(filepath);
    file << "HEADER_MODULE_LIST\n";
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
        file << "posx=" << it->second->entity->position.x << "\n";
        file << "posy=" << it->second->entity->position.y << "\n";
        file << "posz=" << it->second->entity->position.z << "\n";
        file << "scale=" << it->second->entity->scale << "\n";
        file << "rotate=" << it->second->entity->rotate << "\n";
    }

    file.close();

    Log::info("\tsaved: %s\n", filepath);
    Log::info("=== END: SaveModuleListInTextFormat\n");

    return 0;
}

inline int32 SaveGroundListInTextFormat(const char *filepath, std::map<uint32, Ground*> *List)
{
    Log::info("=== BEGIN: SaveGroundListInTextFormat\n");

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

    Log::info("\tsaved: %s\n", filepath);
    Log::info("=== END: SaveGroundListInTextFormat\n");

    return 0;
}

inline int32 SaveSkyboxListInTextFormat(const char *filepath, std::map<uint32, Skybox*> *List)
{
    Log::info("=== BEGIN: SaveSkyboxListInTextFormat\n");

    std::ofstream file;
    file.open(filepath);
    file << "HEADER_SKYBOX_LIST\n";
    for (auto it = List->begin(); it != List->end(); it++)
    {
        file << "###\n";
        file << "id=" << it->first << "\n";
        file << "name=" << it->second->name << "\n";
        file << "directory=" <<  it->second->directory << "\n";
    }

    file.close();

    Log::info("\tsaved: %s\n", filepath);
    Log::info("=== END: SaveSkyboxListInTextFormat\n");

    return 0;
}

inline int32 OpenModuleListFromFile(const char *filepath, std::map<uint32, Module*> *List)
{
    Log::info("=== BEGIN: OpenModuleListFromFile\n");

    std::ifstream file(filepath);
    if (!file.is_open())
    {
        Log::error("\topen file error!\n");
        return -1;
    }

    // Clear map
    for (auto it = List->begin(); it != List->end(); it++)
        delete it->second;
    List->clear();


    uint32 id = 1;
    std::string name = "unknown";
    std::string fullpath = "";
    glm::vec3 position = glm::vec3(0.0f);
    float32 scale = 0.0f;
    float32 rotate = 0.0f;
    
    uint32 fetch = 0;
    uint32 nbFields = 9;
    std::string line;

    std::getline(file, line);
    if (line != "HEADER_MODULE_LIST")
    {
        Log::warn("\tfail to read %s: header is missing\n", filepath);
        Log::info("=== END: OpenModuleListFromFile\n");
        return -1;
    }

    while (std::getline(file, line))
    {
        line.erase(std::remove_if(line.begin(), line.end(), isspace),
                   line.end());

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
            // in case of no objfile
            if (fullpath.length() == 1)
                fullpath = "";

            Log::info("\tfetching fields [%d]/[%d]\n", fetch, nbFields);

            char *nameCopy = new char[32];
            strncpy(nameCopy, name.c_str(), 32);
            nameCopy[32 - 1] = '\0';

            Module *module = new Module(nameCopy, fullpath);
            //entity->pickingSphere =  new Mesh(0.0f, 0.2f, 15, 15);
            module->entity->position = position;
            module->entity->scale = scale;
            module->entity->rotate = rotate;
            List->insert({id, module});
		
            fetch = 0;
        }
    }

    file.close();


    Log::info("\topened: %s\n", filepath);
    Log::info("=== END: OpenModuleListFromFile\n");

    return 0;
}

inline int32 OpenGroundListFromFile(const char *filepath, std::map<uint32,Ground*> *List)
{
    Log::info("=== BEGIN: OpenGroundListFromFile\n");

    std::ifstream file(filepath);
    if (!file.is_open())
    {
        Log::error("\topen file error!\n");
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
        Log::warn("\tfail to read %s: header is missing\n", filepath);
        Log::info("=== END: OpenGroundListFromFile\n");
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

                Log::info("\tfetching fields [%d]/[%d]\n", fetch, nbFields);

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

    Log::info("\topened: %s\n", filepath);
    Log::info("=== END: OpenGroundListFromFile\n");

    return 0;
}

inline int32 OpenSkyboxListFromFile(const char *filepath, std::map<uint32,Skybox*> *List)
{
    Log::info("=== BEGIN: OpenSkyboxListFromFile\n");

    std::ifstream file(filepath);
    if (!file.is_open())
    {
        Log::error("\topen file error!\n");
        return -1;
    }

    // Clear map
    for (auto it = List->begin(); it != List->end(); it++)
        delete it->second;
    List->clear();


    uint32 id = 1;
    std::string name = "unknown";
    std::string directory = "unknown";

    uint32 fetch = 0;
    uint32 nbFields = 3;
    std::string line;

    std::getline(file, line);
    if (line != "HEADER_SKYBOX_LIST")
    {
        Log::warn("\tfail to read %s: header is missing\n", filepath);
        Log::info("=== END: OpenSkyboxListFromFile\n");
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
                directory = value ;
                fetch++;
            }

            if (fetch == nbFields)
            {
                Log::info("\tfetching fields [%d]/[%d]\n", fetch, nbFields);

                char *nameCopy = new char[32];
                strncpy(nameCopy, name.c_str(), 32);
                nameCopy[32 - 1] = '\0';

                Skybox *skybox = new Skybox(nameCopy, directory);
                List->insert({id, skybox});
		
                fetch = 0;
            }            
        }
    }

    file.close();

    Log::info("\topened: %s\n", filepath);
    Log::info("=== END: OpenSkyboxListFromFile\n");

    return 0;
}
