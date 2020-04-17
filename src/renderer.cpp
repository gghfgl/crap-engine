#include "renderer.h"
#include "shader.h"

namespace renderer
{
    renderer_t* Construct()
    {
	// // TODO: think about memory pool after Model loader
	// vertex *CubeBuffer = new vertex[globalMaxVertexCount];
	// memory_arena *Arena = new memory_arena();
	// InitMemoryArena(Arena, sizeof(vertex) * globalMaxVertexCount, (int64*)CubeBuffer);

        renderer_t* Renderer = new renderer_t;
	Renderer->WireframeMode = false;
	return Renderer;
    }

    void Delete(renderer_t *Renderer)
    {
	// ===================== platform code =====================
	// // TODO: move to mesh model etc ...
	// glDeleteVertexArrays(1, &Renderer->VAO->Id);
	// glDeleteBuffers(1, &Renderer->DebugVBO);
	// glDeleteVertexArrays(1, &Renderer->CubeVAO);
	// glDeleteBuffers(1, &Renderer->CubeVBO);
	// glDeleteBuffers(1, &Renderer->CubeIBO);
	// ========================================================

	// TODO: delete MemPool
        delete Renderer;
    }

// TODO: add model in arguments ?
    void NewRenderingContext(renderer_t *Renderer, shader_t *Shader, glm::mat4 viewMatrix)
    {
	// ===================== platform code =====================
	glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// ========================================================

	glm::mat4 model = glm::mat4(1.0f);
	//model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));

	shader::UseProgram(Shader);
	shader::SetUniform4fv(Shader, "view", viewMatrix);    
	shader::SetUniform4fv(Shader, "model", model);
    }

    void RefreshMemoryPool(renderer_t *Renderer)
    {
	Renderer->MemPool->MaxUsed = 0;
    }

    void RendererResetStats(renderer_t *Renderer)
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

    void DrawLines(renderer_t *Renderer, mesh_t *Mesh)
    {
	glBindVertexArray(Mesh->VAO);
	glDrawArrays(GL_LINES, 0, Mesh->VertexCount);
	//Renderer->Stats.DrawCall++;
    }
}
