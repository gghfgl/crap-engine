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

void Renderer::drawLines(Mesh *mesh, float32 width)
{
    glLineWidth(width);
    glBindVertexArray(mesh->VAO);
    glDrawArrays(GL_LINES, 0, (GLsizei)mesh->Vertices.size());

    glBindVertexArray(0);         // good practice
    this->stats.drawCalls++;
}

void Renderer::drawMesh(Mesh *mesh, Shader *shader)
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

        shader->setUniform1i((name + number).c_str(), i);
        glBindTexture(GL_TEXTURE_2D, mesh->Textures[i].ID);
    }

    glBindVertexArray(mesh->VAO);
    glDrawElements(GL_TRIANGLES, (GLsizei)mesh->Indices.size(), GL_UNSIGNED_INT, NULL);

    glBindVertexArray(0);         // good practice
    glActiveTexture(GL_TEXTURE0); // good practice
    this->stats.drawCalls++;
}
    
void Renderer::drawInstanceMesh(Mesh *mesh, Shader *shader, uint32 count)
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

        shader->setUniform1i((name + number).c_str(), i);
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

void Renderer::drawModel(Model *model, Shader *shader)
{
    for (uint32 i = 0; i < model->Meshes.size(); i++)
        this->drawMesh(model->Meshes[i], shader);
}

void Renderer::drawInstanceModel(Model *model, Shader *shader, uint32 count)
{
    for (uint32 i = 0; i < model->Meshes.size(); i++)
        this->drawInstanceMesh(model->Meshes[i], shader, count);
}
    
void Renderer::drawSkybox(Skybox *skybox)
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

void Renderer::prepareOriginDebug(Mesh *mesh)
{    
    mesh->Vertices.clear();

    Vertex vXa;
    vXa.position = glm::vec3(0.0f, 0.1f, 0.0f);
    mesh->Vertices.push_back(vXa);
    Vertex vXb;
    vXb.position = glm::vec3(2.0f, 0.1f, 0.0f);
    mesh->Vertices.push_back(vXb);

    Vertex vYa;
    vYa.position = glm::vec3(0.0f, 0.1f, 0.0f);
    mesh->Vertices.push_back(vYa);
    Vertex vYb;
    vYb.position = glm::vec3(0.0f, 2.0f, 0.0f);
    mesh->Vertices.push_back(vYb);

    Vertex vZa;
    vZa.position = glm::vec3(0.0f, 0.1f, 0.0f);
    mesh->Vertices.push_back(vZa);
    Vertex vZb;
    vZb.position = glm::vec3(0.0f, 0.1f, -2.0f);
    mesh->Vertices.push_back(vZb);
    
    glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO);
    glBufferSubData(GL_ARRAY_BUFFER,
                    0,
                    mesh->Vertices.size() * sizeof(Vertex),
                    &mesh->Vertices[0]);
}

void Renderer::prepareReferenceGridSubData(Mesh *mesh, uint32 resolution)
{
    uint32 vCount = resolution * 4 + 4;			   // 44
    float32 b = (float32)resolution / 2.0f + 1.0f; // 6
    float32 a = -b;								   // -6
    float32 xPos = -((float32)resolution / 2.0f);  // -5
    float32 zPos = xPos;						   // -5

    mesh->Vertices.clear();
    uint32 i = 0;
    while (i < vCount / 2) // z axis ->
    {
        Vertex v;
        if (i % 2 == 0)
        {
            v.position = glm::vec3(a, 0.0f, zPos);
        }
        else
        {
            v.position = glm::vec3(b, 0.0f, zPos);
            zPos += 1.0f;
        }

        mesh->Vertices.push_back(v);
        i++;
    }

    while (i < vCount) // x axis ->
    {
        Vertex v;
        if (i % 2 == 0)
        {
            v.position = glm::vec3(xPos, 0.0f, a);
        }
        else
        {
            v.position = glm::vec3(xPos, 0.0f, b);
            xPos += 1.0f;
        }

        mesh->Vertices.push_back(v);
        i++;
    }
        
    glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO);
    glBufferSubData(GL_ARRAY_BUFFER,
                    0,
                    mesh->Vertices.size() * sizeof(Vertex),
                    &mesh->Vertices[0]);
}

void Renderer::prepareRaySubData(Mesh *mesh, glm::vec3 origin, glm::vec3 direction)
{
    glm::vec3 target = origin + (direction * 1.0f);

    mesh->Vertices.clear();
    Vertex v;
    v.position = glm::vec3(origin.x, origin.y, origin.z - 0.1f);
    mesh->Vertices.push_back(v);
    v.position = target;
    mesh->Vertices.push_back(v);

    glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO);
    glBufferSubData(GL_ARRAY_BUFFER,
                    0,
                    mesh->Vertices.size() * sizeof(Vertex),
                    &mesh->Vertices[0]);
}
