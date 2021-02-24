#include "entity.h"

Entity::~Entity()
{
    delete this->model;
}

void Entity::UpdatePositionFromDirection(EntityDirection direction, float32 deltaTime, float32 acceleration = 0.5f)
{
    float32 velocity = speed * acceleration * deltaTime;
    if (direction == ENTITY_FORWARD)
        position -= glm::vec3(0.0f, 0.0f, 1.0f) * velocity;
    if (direction == ENTITY_BACKWARD)
        position += glm::vec3(0.0f, 0.0f, 1.0f) * velocity;
    if (direction == ENTITY_LEFT)
        position -= glm::vec3(1.0f, 0.0f, 0.0f) * velocity;
    if (direction == ENTITY_RIGHT)
        position += glm::vec3(1.0f, 0.0f, 0.0f) * velocity;
}

// TODO: could be generic
void Entity::UpdateRotationFollowVec(glm::vec3 followWorld, glm::vec2 followScreen, float32 farPlane)
{
    // Compute angle between two position
    glm::vec3 origin = position;
    glm::vec3 A = glm::vec3(position.x, 0.0f, farPlane);
    glm::vec3 B = followWorld;
    glm::vec3 da=glm::normalize(A-origin);
    glm::vec3 db=glm::normalize(B-origin);
    float32 angle = glm::acos(glm::dot(da, db));

    // Convert from radian to degres
    angle = angle * 180 / M_PI;

    // Handling 180 -> 360 decrease angle
    if (followScreen.x > 720)
        rotate = angle;
    else
        rotate = -angle;
}

// ======================================

Ground::Ground(const char* name, std::string const &modelFilePath, uint32 resolution=10)
{
    this->entity = new Entity;

    this->entity->model = nullptr;
    if (modelFilePath.length() > 0)
        this->entity->model = new Model(modelFilePath); // TODO: handler error and delete model in case of failure to ensure "unknown" label from GUI.

    this->resolutionBuffer = (int)resolution;
    this->instanceBufferID = 0;
    this->name = name;

    this->UpdateModelMatrices();

    this->resolution = 0; // reset resolution to trigger the diffResolution at the first loop
}

Ground::~Ground()
{
    delete[] this->name;
    delete this->entity;
    this->ClearInstance();
}

void Ground::UpdateModelMatrices()
{
    // NOTE: every ground should have the same fixed size of half of a module
    const float32 posModifier = 0.25f;

    uint32 resolution = (uint32)this->resolutionBuffer;
    this->resolution = resolution;
    this->modelMatrices = new glm::mat4[resolution * resolution];    

    glm::vec3 size = { 1.0f, 1.0f, 1.0f };
    float32 posX = -((float32)resolution / 2.0f - 0.5f);

    uint32 index = 0;
    for (uint32 i = 0; i < resolution; i++)
    {
        float32 posZ = ((float32)resolution / 2.0f - 0.5f);
        for (uint32 y = 0; y < resolution; y++)
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, { posX, 0.0f + posModifier, posZ });
            model = glm::scale(model, glm::vec3(1.0f));
            model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            this->modelMatrices[index] = model;

            posZ -= size.z;
            index++;
        }
        posX += size.x;
    }
}

void Ground::ClearInstance()
{
    delete[] this->modelMatrices;
    glDeleteBuffers(1, &this->instanceBufferID);
}

bool Ground::DiffResolutionBuffer()
{
    if ((int)this->resolution != this->resolutionBuffer)
        return true;

    return false;
}

// ======================================

Module::Module(const char* name, std::string const &modelFilePath)
{
    this->entity = new Entity;

    this->entity->model = nullptr;
    if (modelFilePath.length() > 0)
        this->entity->model = new Model(modelFilePath); // TODO: handle error and delete model in case of failure to ensure "unknown" label from GUI.

    this->name = name;
}

Module::~Module()
{
    delete[] this->name;
    delete this->entity;
}

// ======================================

Player::Player(const char* name, std::string const &modelFilePath, glm::vec3 position)
{
    this->entity = new Entity;
    this->entity->position = position;

    this->entity->model = nullptr;
    if (modelFilePath.length() > 0)
        this->entity->model = new Model(modelFilePath); // TODO: handle error and delete model in case of failure to ensure "unknown" label from GUI.

    this->name = name;
}

Player::~Player()
{
    delete[] this->name;
    delete this->entity;
}

// ======================================

Skybox::Skybox(const char* name, std::string directoryPath)
{
    auto p = std::filesystem::proximate(directoryPath);
    std::string p_string{p.u8string()};

    float32 skyboxVertices[] = {
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
    this->textureID = 0;
    this->name = name;
    this->directory = p_string;

    if (directoryPath.length() > 0)
        this->LoadCubeMapTextureFromFile(directoryPath);
}

Skybox::~Skybox()
{
    delete[] this->name;
    glDeleteVertexArrays(1, &this->VAO);
    glDeleteBuffers(1, &this->VBO);
    glDeleteTextures(1, &this->textureID);
}

void Skybox::LoadCubeMapTextureFromFile(std::string directoryPath)
{
    std::vector<std::string> faces{
        directoryPath + "/right.jpg",
        directoryPath + "/left.jpg",
        directoryPath + "/top.jpg",
        directoryPath + "/bottom.jpg",
        directoryPath + "/front.jpg",
        directoryPath + "/back.jpg"};
    
    glGenTextures(1, &this->textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, this->textureID);

    int width, height, nrChannels;
    for (uint32 i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        else
        {
            Log::warn("\tCubemap texture failed to load at path: %s\n", faces[i].c_str());
            return;
        }

        stbi_image_free(data);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    auto p = std::filesystem::proximate(directoryPath);
    std::string p_string{p.u8string()};
    this->directory = p_string;
}
