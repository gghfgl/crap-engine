#include "renderer.h"

renderer* RendererConstruct(shader *DefaultShader, shader *StencilShader)
{
    renderer* Renderer = new renderer();
    // TODO: Renderer->Shaders = shaders[] ...
    Renderer->Shader = DefaultShader;
    Renderer->Stencil = StencilShader;
    return Renderer;
}

void RendererDelete(renderer *Renderer)
{
    glDeleteVertexArrays(1, &Renderer->DebugVAO);
    glDeleteBuffers(1, &Renderer->DebugVBO);
    glDeleteVertexArrays(1, &Renderer->CubeVAO);
    glDeleteBuffers(1, &Renderer->CubeVBO);
    glDeleteBuffers(1, &Renderer->CubeIBO);

    // delete texture
    // delete cubebufferptr?
    delete[] Renderer->CubeBuffer;
    delete Renderer;
}

void RendererPrepareDebugAxis(renderer *Renderer)
{    
    float debug_axis[] =
	{
	    0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
	    3.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,

	    0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
	    0.0f, 3.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
	
	    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,
	    0.0f, 0.0f, -3.0f, 0.0f, 0.0f, 1.0f, 1.0f
	};

    glGenVertexArrays(1, &Renderer->DebugVAO);
    glBindVertexArray(Renderer->DebugVAO);

    glGenBuffers(1, &Renderer->DebugVBO);
    glBindBuffer(GL_ARRAY_BUFFER, Renderer->DebugVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(debug_axis), debug_axis, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float32), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float32), (void*)(3 * sizeof(float32)));
}

void RendererDrawDebugAxis(renderer *Renderer)
{
    glBindVertexArray(Renderer->DebugVAO);
    glDrawArrays(GL_LINES, 0, 6);
    Renderer->Stats.DrawCount++;
}

void RendererStart(renderer *Renderer, glm::mat4 viewMatrix)
{
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glm::mat4 model = glm::mat4(1.0f);
    //model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));

    ShaderUseProgram(Renderer->Shader);
    ShaderSetUniform4fv(Renderer->Shader, "view", viewMatrix);    
    ShaderSetUniform4fv(Renderer->Shader, "model", model);

    glStencilMask(0x00); // dont update the stencil buffer
}

void RendererStartStencil(renderer *Renderer, glm::mat4 viewMatrix)
{
    // NOTE: assume that ClearColor was made before
    float32 scale = 1.1f;
    glm::mat4 model = glm::mat4(1.0f);
    //model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
    //model = glm::scale(model, glm::vec3(scale, scale, scale));

    ShaderUseProgram(Renderer->Stencil);
    ShaderSetUniform4fv(Renderer->Stencil, "view", viewMatrix);    
    ShaderSetUniform4fv(Renderer->Stencil, "model", model);

    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilMask(0x00);
    glDisable(GL_DEPTH_TEST);
}

void RendererStopStencil()
{
    glStencilMask(0xFF);
    glDisable(GL_STENCIL_TEST);
}

void RendererSwapBufferAndFinish(GLFWwindow *Window)
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST); // TODO: do some check?
    glfwSwapBuffers(Window);
    glFinish();
}

void RendererPrepareCubeBatching(renderer *Renderer)
{
    Renderer->CubeBuffer = new vertex[globalMaxVertexCount];

    glGenVertexArrays(1, &Renderer->CubeVAO);
    glBindVertexArray(Renderer->CubeVAO);

    glGenBuffers(1, &Renderer->CubeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, Renderer->CubeVBO);
    // DYNAMIC because of no data initialization and set subdata every frame later
    glBufferData(GL_ARRAY_BUFFER, globalMaxVertexCount * sizeof(vertex), nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (const void*)offsetof(vertex, Position));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(vertex), (const void*)offsetof(vertex, Color));

    // predictable cube layout
    uint32 indices[globalMaxIndexCount];
    uint32 offset = 0;
    for (uint32 i = 0; i < globalMaxIndexCount; i += globalPerCubeIndices)
    { // 8 = nb of vertex needed for 1 cube
	indices[i + 0] = 0 + offset;
	indices[i + 1] = 1 + offset;
	indices[i + 2] = 2 + offset;
	indices[i + 3] = 2 + offset;
    	indices[i + 4] = 3 + offset;
	indices[i + 5] = 0 + offset;

	indices[i + 6] = 1 + offset;
	indices[i + 7] = 5 + offset;
	indices[i + 8] = 6 + offset;
	indices[i + 9] = 6 + offset;
    	indices[i + 10] = 2 + offset;
	indices[i + 11] = 1 + offset;

	indices[i + 12] = 7 + offset;
	indices[i + 13] = 6 + offset;
	indices[i + 14] = 5 + offset;
	indices[i + 15] = 5 + offset;
    	indices[i + 16] = 4 + offset;
	indices[i + 17] = 7 + offset;

	indices[i + 18] = 4 + offset;
	indices[i + 19] = 0 + offset;
	indices[i + 20] = 3 + offset;
	indices[i + 21] = 3 + offset;
    	indices[i + 22] = 7 + offset;
	indices[i + 23] = 4 + offset;

	indices[i + 24] = 4 + offset;
	indices[i + 25] = 5 + offset;
	indices[i + 26] = 1 + offset;
	indices[i + 27] = 1 + offset;
    	indices[i + 28] = 0 + offset;
	indices[i + 29] = 4 + offset;

	indices[i + 30] = 3 + offset;
	indices[i + 31] = 2 + offset;
	indices[i + 32] = 6 + offset;
	indices[i + 33] = 6 + offset;
    	indices[i + 34] = 7 + offset;
	indices[i + 35] = 3 + offset;

	offset += globalPerCubeVertex;
    }

    glGenBuffers(1, &Renderer->CubeIBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Renderer->CubeIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
}

void RendererResetStats(renderer *Renderer)
{
    // Set all values as 0 for integral data type (byte by byte)
    memset(&Renderer->Stats, 0, sizeof(renderer_stats));
}

void RendererStartNewBatchCube(renderer *Renderer)
{
    Renderer->CubeBufferPtr = Renderer->CubeBuffer;
}

void RendererCloseBatchCube(renderer *Renderer)
{
    GLsizeiptr size = (uint8*)Renderer->CubeBufferPtr - (uint8*)Renderer->CubeBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, Renderer->CubeVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, size, Renderer->CubeBuffer);
}

void RendererFlushBatchCube(renderer *Renderer)
{
    glBindVertexArray(Renderer->CubeVAO);
    glDrawElements(GL_TRIANGLES, Renderer->IndexCount, GL_UNSIGNED_INT, nullptr);

    Renderer->Stats.DrawCount++;
    Renderer->IndexCount = 0;
}

// TODO: PrepareSphereBatchRendering()

// TODO: AddSphereToBuffer()

// TODO: StartNewBatchSphere()

// TODO: CloseBatchSphere()

// TODO: FlushBatchSphere()

void RendererAddCubeToBuffer(renderer *Renderer,
		     const glm::vec3 &position,
		     const glm::vec3 &size,
		     const float32 &scale,
		     const glm::vec4 &color)
{    
    // Are we out of vertex buffer? if then reset everything
    if (Renderer->IndexCount >= globalMaxIndexCount)
    {
        RendererCloseBatchCube(Renderer);
        RendererFlushBatchCube(Renderer);
        RendererStartNewBatchCube(Renderer);
    }

    float32 posX = position.x + 1.0f / 2.0f - scale / 2.0f;
    float32 posY = position.y + 1.0f / 2.0f - scale / 2.0f;
    float32 posZ = position.z + 1.0f / 2.0f - scale / 2.0f;
    
    // FRONT
    Renderer->CubeBufferPtr->Position = { posX, posY, posZ };
    Renderer->CubeBufferPtr->Color = color;
    Renderer->CubeBufferPtr++;

    Renderer->CubeBufferPtr->Position = { posX + size.x * scale, posY, posZ };
    Renderer->CubeBufferPtr->Color = color;
    Renderer->CubeBufferPtr++;

    Renderer->CubeBufferPtr->Position = { posX + size.x * scale, posY + size.y * scale, posZ };
    Renderer->CubeBufferPtr->Color = color;
    Renderer->CubeBufferPtr++;

    Renderer->CubeBufferPtr->Position = { posX, posY + size.y * scale, posZ };
    Renderer->CubeBufferPtr->Color = color;
    Renderer->CubeBufferPtr++;

    // BACK
    Renderer->CubeBufferPtr->Position = { posX, posY, posZ + size.z * scale };
    Renderer->CubeBufferPtr->Color = color;
    Renderer->CubeBufferPtr++;

    Renderer->CubeBufferPtr->Position = { posX + size.x * scale, posY, posZ + size.z * scale };
    Renderer->CubeBufferPtr->Color = color;
    Renderer->CubeBufferPtr++;

    Renderer->CubeBufferPtr->Position = { posX + size.x * scale, posY + size.y * scale, posZ + size.z * scale };
    Renderer->CubeBufferPtr->Color = color;
    Renderer->CubeBufferPtr++;

    Renderer->CubeBufferPtr->Position = { posX, posY + size.y * scale, posZ + size.z * scale };
    Renderer->CubeBufferPtr->Color = color;
    Renderer->CubeBufferPtr++;

    Renderer->IndexCount += globalPerCubeIndices; // 36
    Renderer->Stats.CubeCount++;
}

static void RendererSettingsCollapseHeader(renderer *Renderer)
{
    if (ImGui::CollapsingHeader("Render settings", ImGuiTreeNodeFlags_DefaultOpen))
    {
    	ImGui::Text("maxCube: %d", globalMaxCubeCount);
    	ImGui::Text("current: %d", Renderer->Stats.CubeCount);
    	ImGui::Text("draw: %d", Renderer->Stats.DrawCount);
    	ImGui::Separator();
    }
}
