#include "renderer.h"

namespace renderer
{
    renderer_t* Construct()
    {
	// // TODO: think about memory pool after Model loader
	// vertex *CubeBuffer = new vertex[globalMaxVertexCount];
	// memory_arena *Arena = new memory_arena();
	// InitMemoryArena(Arena, sizeof(vertex) * globalMaxVertexCount, (int64*)CubeBuffer);

	renderer_t* Renderer = new renderer_t;
	Renderer->PolygoneMode = false;
	return Renderer;
    }

// TODO: malloc?
    void Delete(renderer_t *Renderer)
    {
	// ===================== platform code =====================
	// // TODO: delete cascade
	// glDeleteVertexArrays(1, &Renderer->DebugVAO);
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

    void TogglePolygoneMode(renderer_t *Renderer)
    {
	Renderer->PolygoneMode = !Renderer->PolygoneMode;
	// ===================== platform code =====================
	glPolygonMode(GL_FRONT_AND_BACK, (Renderer->PolygoneMode ? GL_FILL : GL_LINE));
	// ========================================================
    }

// TODO: use for moving / scaling model ?
// void RendererPrepareDebugAxis(renderer *Renderer)
// {    
//     float debug_axis[] =
// 	{
// 	    0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
// 	    3.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,

// 	    0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
// 	    0.0f, 3.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
	
// 	    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,
// 	    0.0f, 0.0f, -3.0f, 0.0f, 0.0f, 1.0f, 1.0f
// 	};

//     glGenVertexArrays(1, &Renderer->DebugVAO);
//     glBindVertexArray(Renderer->DebugVAO);

//     glGenBuffers(1, &Renderer->DebugVBO);
//     glBindBuffer(GL_ARRAY_BUFFER, Renderer->DebugVBO);
//     glBufferData(GL_ARRAY_BUFFER, sizeof(debug_axis), debug_axis, GL_STATIC_DRAW);

//     glEnableVertexAttribArray(0);
//     glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float32), (void*)0);

//     glEnableVertexAttribArray(1);
//     glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float32), (void*)(3 * sizeof(float32)));
// }

// void RendererDrawDebugAxis(renderer *Renderer)
// {
//     glBindVertexArray(Renderer->DebugVAO);
//     glDrawArrays(GL_LINES, 0, 6);
//     Renderer->Stats.DrawCount++;
// }
}
