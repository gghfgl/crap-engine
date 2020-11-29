#include "entity.h"

static glm::mat4* generate_terrain_model_matrices(uint32 sideLenght);
static uint32 load_cubemap_texture_from_file(std::vector<std::string> faces);

void Delete(entity_t *Entity)
{
    Delete(Entity->Model);
    if (Entity->PickingSphere != nullptr)
        Delete(Entity->PickingSphere);
    delete Entity;
}

terrain_t* AllocAndInit(uint32 resolution, glm::vec3 unitSize, std::string const &modelFilePath)
{
    terrain_t *Terrain = new terrain_t;
    Terrain->Resolution = resolution;
    Terrain->UnitSize = unitSize;
    Terrain->Entity = new entity_t;
    Terrain->Entity->PickingSphere = nullptr;
    Terrain->Entity->Model = LoadModelFromFile(modelFilePath);

    return Terrain;
}

void CleanInstance(terrain_t *Terrain)
{
    delete[] Terrain->ModelMatrices;
    glDeleteBuffers(1, &Terrain->InstanceBufferID);
}

void Delete(terrain_t *Terrain)
{
    Delete(Terrain->Entity);
    delete[] Terrain->ModelMatrices;
    delete Terrain;
}

glm::mat4* GenerateTerrainModelMatrices(uint32 sideLenght)
{
    glm::mat4 *modelMatrices;
    modelMatrices = new glm::mat4[sideLenght * sideLenght];    
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
            model = glm::rotate(model, glm::radians(0.0f),
                                glm::vec3(0.0f, 1.0f, 0.0f));
            modelMatrices[index] = model;

            posZ -= size.z;
            index++;
        }
        posX += size.x;
    }

    return modelMatrices;
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
