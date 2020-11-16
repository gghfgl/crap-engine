#include "renderer.h"
#include "shader.h" // TODO: in header file?

Renderer::Renderer()
{
    // // TODO: think about memory pool after Model loader
    // vertex *CubeBuffer = new vertex[globalMaxVertexCount];
    // memory_arena *Arena = new memory_arena();
    // InitMemoryArena(Arena, sizeof(vertex) * globalMaxVertexCount, (int64*)CubeBuffer);

    this->wireframe = false;
    this->stats.drawCalls = 0;
    this->stats.vertexCount = 0;
}

Renderer::~Renderer()
{
    // TODO: delete MemPool
}

void Renderer::newContext()
{
    // ===================== platform code =====================
    glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // ========================================================
}

void Renderer::resetStats()
{
    this->stats.drawCalls = 0;
    this->stats.vertexCount = 0;
}

// TODO: use geometry shader to make wireframe effect on individual object
void Renderer::toggleWireframe()
{
    this->wireframe = !this->wireframe;

    // ===================== platform code =====================
    glPolygonMode(GL_FRONT_AND_BACK, (this->wireframe ? GL_FILL : GL_LINE));
    // ========================================================
}

uint32 Renderer::prepareInstance(Model *model,
                                 glm::mat4 *modelMatrices,
                                 uint32 count)
{
    uint32 buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER,
                 count * sizeof(glm::mat4),
                 &modelMatrices[0],
                 GL_STATIC_DRAW);

    for (uint32 i = 0; i < model->Meshes.size(); i++)
    {
        glBindVertexArray(model->Meshes[i]->VAO);

        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 4,
                              GL_FLOAT, GL_FALSE,
                              sizeof(glm::mat4), (void*)0);
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4,
                              GL_FLOAT, GL_FALSE,
                              sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
        glEnableVertexAttribArray(7);
        glVertexAttribPointer(7, 4,
                              GL_FLOAT, GL_FALSE,
                              sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
        glEnableVertexAttribArray(8);
        glVertexAttribPointer(8, 4,
                              GL_FLOAT, GL_FALSE,
                              sizeof(glm::mat4),(void*)(3 * sizeof(glm::vec4)));

        glVertexAttribDivisor(5, 1);
        glVertexAttribDivisor(6, 1);
        glVertexAttribDivisor(7, 1);
        glVertexAttribDivisor(8, 1);

        glBindVertexArray(0);
    }

    return buffer;
}

void Renderer::drawLines(Mesh *mesh, float32 width, shader_t *shader)
{
    glLineWidth(width);
    glBindVertexArray(mesh->VAO);
    glDrawArrays(GL_LINES, 0, (GLsizei)mesh->Vertices.size());

    glBindVertexArray(0);         // good practice
    this->stats.drawCalls++;
}

void Renderer::drawMesh(Mesh *mesh, shader_t *shader)
{
    uint32 diffuseNr = 1;
    uint32 specularNr = 1;
    uint32 normalNr = 1;
    uint32 heightNr = 1;

    // TODO: improve texture management
    for (uint32 i = 0; i < mesh->Textures.size(); i++)
    {
        glActiveTexture(GL_TEXTURE0 + i); // active proper texture unit before binding
        std::string number;
        std::string name = mesh->Textures[i].type;
        if (name == "texture_diffuse")
            number = std::to_string(diffuseNr++);
        else if (name == "texture_specular")
            number = std::to_string(specularNr++); // transfer unsigned int to stream
        else if (name == "texture_normal")
            number = std::to_string(normalNr++); // transfer unsigned int to stream
        else if (name == "texture_height")
            number = std::to_string(heightNr++); // transfer unsigned int to stream

        SetUniform1i(shader, (name + number).c_str(), i);
        glBindTexture(GL_TEXTURE_2D, mesh->Textures[i].ID);
    }

    glBindVertexArray(mesh->VAO);
    glDrawElements(GL_TRIANGLES, (GLsizei)mesh->Indices.size(), GL_UNSIGNED_INT, NULL);

    glBindVertexArray(0);         // good practice
    glActiveTexture(GL_TEXTURE0); // good practice
    this->stats.drawCalls++;
}
    
void Renderer::drawInstanceMesh(Mesh *mesh, shader_t *shader, uint32 count)
{
    uint32 diffuseNr = 1;
    uint32 specularNr = 1;
    uint32 normalNr = 1;
    uint32 heightNr = 1;

    // TODO: improve texture management
    for (uint32 i = 0; i < mesh->Textures.size(); i++)
    {
        glActiveTexture(GL_TEXTURE0 + i); // active proper texture unit before binding
        std::string number;
        std::string name = mesh->Textures[i].type;
        if (name == "texture_diffuse")
            number = std::to_string(diffuseNr++);
        else if (name == "texture_specular")
            number = std::to_string(specularNr++); // transfer unsigned int to stream
        else if (name == "texture_normal")
            number = std::to_string(normalNr++); // transfer unsigned int to stream
        else if (name == "texture_height")
            number = std::to_string(heightNr++); // transfer unsigned int to stream

        SetUniform1i(shader, (name + number).c_str(), i);
        glBindTexture(GL_TEXTURE_2D, mesh->Textures[i].ID);
    }

    glBindVertexArray(mesh->VAO);
    glDrawElementsInstanced(GL_TRIANGLES,
                            (GLsizei)mesh->Indices.size(),
                            GL_UNSIGNED_INT,
                            0,
                            count);

    glBindVertexArray(0);         // good practice
    glActiveTexture(GL_TEXTURE0); // good practice
    this->stats.drawCalls++;
}

void Renderer::drawModel(Model *model, shader_t *shader)
{
    for (uint32 i = 0; i < model->Meshes.size(); i++)
        this->drawMesh(model->Meshes[i], shader);
}

void Renderer::drawInstanceModel(Model *model, shader_t *shader, uint32 count)
{
    for (uint32 i = 0; i < model->Meshes.size(); i++)
        this->drawInstanceMesh(model->Meshes[i], shader, count);
}
    
void Renderer::drawSkybox(Skybox *skybox, shader_t *shader)
{
    glDepthFunc(GL_LEQUAL);

    glBindVertexArray(skybox->VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skybox->textureID);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    glBindVertexArray(0);
    glDepthFunc(GL_LESS);
    this->stats.drawCalls++;
}
