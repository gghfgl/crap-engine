#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "stb_image.h" // TODO remove
#include "renderer.h"

void GLClearError()
{
    while (glGetError() != GL_NO_ERROR);
}

bool GLLogCall(const char* function, const char* file, int line)
{
    while (GLenum error = glGetError())   
    {
	std::cout << "[OpenGL Error] (" << error << "): "
		  << function << " " << file << ":" << line
		  << std::endl;
	return false;
    }
    return true;
}

renderer* RendererConstruct(shader *Shader)
{
    renderer* Result = new renderer();
    Result->Shader = Shader;
    return Result;
}

void PrepareEmbededAxisDebugRendering(renderer *Renderer)
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

    GLCall(glGenVertexArrays(1, &Renderer->DebugVAO));
    GLCall(glBindVertexArray(Renderer->DebugVAO));

    GLCall(glGenBuffers(1, &Renderer->DebugVBO));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, Renderer->DebugVBO));
    GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(debug_axis), debug_axis, GL_STATIC_DRAW));

    GLCall(glEnableVertexAttribArray(0));
    GLCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0));

    GLCall(glEnableVertexAttribArray(1));
    GLCall(glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float))));
}

void DrawDebug(renderer *Renderer)
{
    glBindVertexArray(Renderer->DebugVAO);
    glDrawArrays(GL_LINES, 0, 6);
    Renderer->Stats.DrawCount++;
}

void PrepareCubeBatchRendering(renderer *Renderer)
{
    Renderer->CubeBuffer = new vertex[MaxVertexCount];

    GLCall(glGenVertexArrays(1, &Renderer->CubeVAO));
    GLCall(glBindVertexArray(Renderer->CubeVAO));

    GLCall(glGenBuffers(1, &Renderer->CubeVBO));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, Renderer->CubeVBO));
    // DYNAMIC because of no data initialization and set data every frame later
    GLCall(glBufferData(GL_ARRAY_BUFFER, MaxVertexCount * sizeof(vertex), nullptr, GL_DYNAMIC_DRAW));

    GLCall(glEnableVertexAttribArray(0));
    GLCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (const void*)offsetof(vertex, Position)));

    GLCall(glEnableVertexAttribArray(1));
    GLCall(glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(vertex), (const void*)offsetof(vertex, Color)));

    // predictable cube layout
    uint32_t indices[MaxIndexCount];
    uint32_t offset = 0;
    for (int i = 0; i < MaxIndexCount; i += PerCubeIndices)
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

	offset += PerCubeVertex;
    }

    GLCall(glGenBuffers(1, &Renderer->CubeIBO));
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Renderer->CubeIBO));
    GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW));
}

void CleanAndDeleteRenderer(renderer *Renderer)
{
    GLCall(glDeleteVertexArrays(1, &Renderer->DebugVAO));
    GLCall(glDeleteBuffers(1, &Renderer->DebugVBO));
    GLCall(glDeleteVertexArrays(1, &Renderer->CubeVAO));
    GLCall(glDeleteBuffers(1, &Renderer->CubeVBO));
    GLCall(glDeleteBuffers(1, &Renderer->CubeIBO));

    // delete texture
    // delete cubebufferptr?
    delete[] Renderer->CubeBuffer;
    delete Renderer;
}

void StartNewCubeBatch(renderer *Renderer)
{
    Renderer->CubeBufferPtr = Renderer->CubeBuffer;
}

void CloseCubeBatch(renderer *Renderer)
{
    GLsizeiptr size = (uint8_t*)Renderer->CubeBufferPtr - (uint8_t*)Renderer->CubeBuffer;
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, Renderer->CubeVBO));
    GLCall(glBufferSubData(GL_ARRAY_BUFFER, 0, size, Renderer->CubeBuffer));
}

void FlushCubeBatch(renderer *Renderer)
{
    GLCall(glBindVertexArray(Renderer->CubeVAO));
    GLCall(glDrawElements(GL_TRIANGLES, Renderer->IndexCount, GL_UNSIGNED_INT, nullptr));

    Renderer->Stats.DrawCount++;
    Renderer->IndexCount = 0;
}

void AddCubeToBuffer(renderer *Renderer,
		     const glm::vec3 &Position,
		     const glm::vec3 &Size,
		     const glm::vec4 &Color)
{    
    // Are we out of vertex buffer? if then reset everything
    if (Renderer->IndexCount >= MaxIndexCount)
    {
        CloseCubeBatch(Renderer);
        FlushCubeBatch(Renderer);
        StartNewCubeBatch(Renderer);
    }

    // FRONT
    Renderer->CubeBufferPtr->Position =
	{ Position.x, Position.y, Position.z };
    Renderer->CubeBufferPtr->Color = Color;
    Renderer->CubeBufferPtr++;

    Renderer->CubeBufferPtr->Position =
	{ Position.x + Size.x, Position.y, Position.z };
    Renderer->CubeBufferPtr->Color = Color;
    Renderer->CubeBufferPtr++;

    Renderer->CubeBufferPtr->Position =
	{ Position.x + Size.x, Position.y + Size.y, Position.z };
    Renderer->CubeBufferPtr->Color = Color;
    Renderer->CubeBufferPtr++;

    Renderer->CubeBufferPtr->Position =
	{ Position.x, Position.y + Size.y, Position.z };
    Renderer->CubeBufferPtr->Color = Color;
    Renderer->CubeBufferPtr++;

    // BACK
    Renderer->CubeBufferPtr->Position =
	{ Position.x, Position.y, Position.z + Size.z };
    Renderer->CubeBufferPtr->Color = Color;
    Renderer->CubeBufferPtr++;

    Renderer->CubeBufferPtr->Position =
	{ Position.x + Size.x, Position.y, Position.z + Size.z };
    Renderer->CubeBufferPtr->Color = Color;
    Renderer->CubeBufferPtr++;

    Renderer->CubeBufferPtr->Position =
	{ Position.x + Size.x, Position.y + Size.y, Position.z + Size.z };
    Renderer->CubeBufferPtr->Color = Color;
    Renderer->CubeBufferPtr++;

    Renderer->CubeBufferPtr->Position =
	{ Position.x, Position.y + Size.y, Position.z + Size.z };
    Renderer->CubeBufferPtr->Color = Color;
    Renderer->CubeBufferPtr++;

    Renderer->IndexCount += PerCubeIndices; // 36
    Renderer->Stats.CubeCount++;
}

void ResetRendererStats(renderer *Renderer)
{
    // Set all values as 0 for integral data type (byte by byte)
    memset(&Renderer->Stats, 0, sizeof(renderer_stats));
}


// void OLD_STUFF(renderer *Renderer)
// {    
//     float vertices[] =
//     {
// 	// front
// 	-1.0f, -1.0f,  1.0, // 0
// 	1.0f, -1.0f,  1.0f, // 1
// 	1.0f,  1.0f,  1.0f, // 2
// 	-1.0f,  1.0f,  1.0f, // 3

// 	// back
// 	-1.0f, -1.0f, -1.0f, // 4
// 	1.0f, -1.0f, -1.0f, // 5
// 	1.0f,  1.0f, -1.0f, // 6
// 	-1.0f,  1.0f, -1.0f, // 7

//     	// front 2
// 	-1.0f, -1.0f,  -1.0, // 8
// 	1.0f, -1.0f,  -1.0f, // 9
// 	1.0f,  1.0f,  -1.0f, // 10
// 	-1.0f,  1.0f,  -1.0f, // 11

// 	// back 2
// 	-1.0f, -1.0f, -3.0f, // 12
// 	1.0f, -1.0f, -3.0f, // 13
// 	1.0f,  1.0f, -3.0f, // 14
// 	-1.0f,  1.0f, -3.0f // 15
//     };

//     unsigned int indices[] =
//	 {
// 	// front
// 	0, 1, 2, 2, 3, 0,
// 	// right
// 	1, 5, 6, 6, 2, 1,
// 	// back
// 	7, 6, 5, 5, 4, 7,
// 	// left
// 	4, 0, 3, 3, 7, 4,
// 	// bottom
// 	4, 5, 1, 1, 0, 4,
// 	// top
// 	3, 2, 6, 6, 7, 3,

//     	// front
// 	8, 9, 10, 10, 11, 8,
// 	// right
// 	9, 13, 14, 14, 10, 9,
// 	// back
// 	15, 14, 13, 13, 12, 15,
// 	// left
// 	12, 8, 11, 11, 15, 12,
// 	// bottom
// 	12, 13, 9, 9, 8, 12,
// 	// top
// 	11, 20, 14, 14, 15, 11
//     };
    
//     GLCall(glGenVertexArrays(1, &Renderer->CubeVAO));
//     GLCall(glBindVertexArray(Renderer->CubeVAO));

//     unsigned int VBO;
//     GLCall(glGenBuffers(1, &VBO));
//     GLCall(glBindBuffer(GL_ARRAY_BUFFER, VBO));
//     GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW));

//     // position attribute
//     GLCall(glEnableVertexAttribArray(0));
//     GLCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0));

//     // // texture coord attribute
//     // GLCall(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float))));
//     // GLCall(glEnableVertexAttribArray(1));

//     unsigned int IBO;
//     GLCall(glGenBuffers(1, &IBO));
//     GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO));
//     GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW));


//     // // load and create a texture 
//     // // -------------------------
//     // // texture 1
//     // // ---------
//     // GLCall(glGenTextures(1, &texture1));
//     // GLCall(glBindTexture(GL_TEXTURE_2D, texture1));
//     // // set the texture wrapping parameters
//     // GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
//     // GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
//     // // set texture filtering parameters
//     // GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
//     // GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
//     // // load image, create texture and generate mipmaps
//     // int width, height, nrChannels;
//     // stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
//     // unsigned char *data = stbi_load("../assets/container2.png", &width, &height, &nrChannels, 0);
//     // if (data)
//     //
//	     {
//     //     GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data));
//     // 	GLCall(glGenerateMipmap(GL_TEXTURE_2D));
//     // }
//     // else
//     //
//		 {
//     //     std::cout << "Failed to load texture" << std::endl;
//     // }
//     // stbi_image_free(data);

//     // // texture 2
//     // // ---------
//     // GLCall(glGenTextures(1, &texture2));
//     // GLCall(glBindTexture(GL_TEXTURE_2D, texture2));
//     // // set the texture wrapping parameters
//     // GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
//     // GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
//     // // set texture filtering parameters
//     // GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
//     // GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
//     // // load image, create texture and generate mipmaps
//     // data = stbi_load("../assets/awesomeface.png", &width, &height, &nrChannels, 0);
//     // if (data)
//     //
//		     {
//     //     // note that the awesomeface.png has transparency and thus an alpha channel, so make sure to tell OpenGL the data type is of GL_RGBA
//     //     GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data));
//     // 	GLCall(glGenerateMipmap(GL_TEXTURE_2D));
//     // }
//     // else
//     //
//			 {
//     //     std::cout << "Failed to load texture" << std::endl;
//     // }
//     // stbi_image_free(data);

//     // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
//     // -------------------------------------------------------------------------------------------
//     //Renderer->shader->Use();
//     //Renderer->shader->SetInteger("texture1", 0);
//     // Renderer->shader->SetInteger("texture2", 1);    
// }
