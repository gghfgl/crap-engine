#include "renderer.h"
#include "shader.h"

namespace renderer
{
    renderer_t *Construct()
    {
        // // TODO: think about memory pool after Model loader
        // vertex *CubeBuffer = new vertex[globalMaxVertexCount];
        // memory_arena *Arena = new memory_arena();
        // InitMemoryArena(Arena, sizeof(vertex) * globalMaxVertexCount, (int64*)CubeBuffer);

        renderer_t *Renderer = new renderer_t;
        Renderer->WireframeMode = false;
        return Renderer;
    }

    void Delete(renderer_t *Renderer)
    {
        // TODO: delete MemPool
        delete Renderer;
    }

    void NewRenderingContext(renderer_t *Renderer)
    {
        // ===================== platform code =====================
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // ========================================================
    }

    void ResetStats(renderer_t *Renderer)
    {
        memset(&Renderer->Stats, 0, sizeof(renderer_stats));
    }

    // TODO: use geometry shader to make wireframe effect on individual object
    void ToggleWireframeMode(renderer_t *Renderer)
    {
        Renderer->WireframeMode = !Renderer->WireframeMode;
        // ===================== platform code =====================
        glPolygonMode(GL_FRONT_AND_BACK, (Renderer->WireframeMode ? GL_FILL : GL_LINE));
        // ========================================================
    }

    void PrepareInstancedRendering(renderer_t *Renderer,
                                   model_t *Model,
                                   glm::mat4 *modelMatrices,
                                   uint32 instanceCount)
    {
        uint32 buffer;
        glGenBuffers(1, &buffer);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glBufferData(GL_ARRAY_BUFFER,
                     instanceCount * sizeof(glm::mat4),
                     &modelMatrices[0],
                     GL_STATIC_DRAW);

        for (uint32 i = 0; i < Model->Meshes.size(); i++)
        {
            glBindVertexArray(Model->Meshes[i]->VAO);

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
    }

    void DrawLines(renderer_t *Renderer, mesh_t *Mesh, shader_t *Shader)
    {
        glBindVertexArray(Mesh->VAO);
        glDrawArrays(GL_LINES, 0, (GLsizei)Mesh->Vertices.size());

        glBindVertexArray(0);         // good practice
        Renderer->Stats.DrawCalls++;
    }

    void DrawMesh(renderer_t *Renderer, mesh_t *Mesh, shader_t *Shader)
    {
        uint32 diffuseNr = 1;
        uint32 specularNr = 1;
        uint32 normalNr = 1;
        uint32 heightNr = 1;

        // TODO: improve texture management
        for (uint32 i = 0; i < Mesh->Textures.size(); i++)
        {
            glActiveTexture(GL_TEXTURE0 + i); // active proper texture unit before binding
            std::string number;
            std::string name = Mesh->Textures[i].Type;
            if (name == "texture_diffuse")
                number = std::to_string(diffuseNr++);
            else if (name == "texture_specular")
                number = std::to_string(specularNr++); // transfer unsigned int to stream
            else if (name == "texture_normal")
                number = std::to_string(normalNr++); // transfer unsigned int to stream
            else if (name == "texture_height")
                number = std::to_string(heightNr++); // transfer unsigned int to stream

            shader::SetUniform1i(Shader, (name + number).c_str(), i);
            glBindTexture(GL_TEXTURE_2D, Mesh->Textures[i].Id);
        }

        glBindVertexArray(Mesh->VAO);
        glDrawElements(GL_TRIANGLES, (GLsizei)Mesh->Indices.size(), GL_UNSIGNED_INT, NULL);

        glBindVertexArray(0);         // good practice
        glActiveTexture(GL_TEXTURE0); // good practice
        Renderer->Stats.DrawCalls++;
    }
    
    void DrawMeshInstanced(renderer_t *Renderer, mesh_t *Mesh, shader_t *Shader, uint32 instanceCount)
    {
        uint32 diffuseNr = 1;
        uint32 specularNr = 1;
        uint32 normalNr = 1;
        uint32 heightNr = 1;

        // TODO: improve texture management
        for (uint32 i = 0; i < Mesh->Textures.size(); i++)
        {
            glActiveTexture(GL_TEXTURE0 + i); // active proper texture unit before binding
            std::string number;
            std::string name = Mesh->Textures[i].Type;
            if (name == "texture_diffuse")
                number = std::to_string(diffuseNr++);
            else if (name == "texture_specular")
                number = std::to_string(specularNr++); // transfer unsigned int to stream
            else if (name == "texture_normal")
                number = std::to_string(normalNr++); // transfer unsigned int to stream
            else if (name == "texture_height")
                number = std::to_string(heightNr++); // transfer unsigned int to stream

            shader::SetUniform1i(Shader, (name + number).c_str(), i);
            glBindTexture(GL_TEXTURE_2D, Mesh->Textures[i].Id);
        }

        glBindVertexArray(Mesh->VAO);
        glDrawElementsInstanced(GL_TRIANGLES,
                                (GLsizei)Mesh->Indices.size(),
                                GL_UNSIGNED_INT,
                                0,
                                instanceCount);

        glBindVertexArray(0);         // good practice
        glActiveTexture(GL_TEXTURE0); // good practice
        Renderer->Stats.DrawCalls++;
    }

    void DrawModel(renderer_t *Renderer, model_t *Model, shader_t *Shader)
    {
        for (uint32 i = 0; i < Model->Meshes.size(); i++)
            DrawMesh(Renderer, Model->Meshes[i], Shader);
    }

    void DrawModelInstanced(renderer_t *Renderer, model_t *Model, shader_t *Shader, uint32 instanceCount)
    {
        for (uint32 i = 0; i < Model->Meshes.size(); i++)
            DrawMeshInstanced(Renderer, Model->Meshes[i], Shader, instanceCount);
    }
    
    void DrawSkybox(renderer_t *Renderer, skybox_t *Skybox, shader_t *Shader)
    {
        glDepthFunc(GL_LEQUAL);

        glBindVertexArray(Skybox->VAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, Skybox->TextureId);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glBindVertexArray(0);
        glDepthFunc(GL_LESS);
        Renderer->Stats.DrawCalls++;
    }
} // namespace renderer
