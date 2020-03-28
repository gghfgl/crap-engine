#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "plateform.h"

/* NOTE: should implement
   - depth test / z-fighting
   - stencil test outline object
   - face culling
   - white / blanc texture
   - blending
   - framebuffer ?
   - mipmap ?
   - cubemap / skybox / reflect ?
   - geometry shader ?
   - instancing ?
   - MSAA anti aliasing ?
   - light system / PBR?
   - load models
*/

// TODO:
// * implement frame rate counter ms VSYNC ?
// * logger ?
// * implement game entity and level?
// * in fuction time profiler (handemade hero fast thread id retrieval)
// * work on z-buffer to avoid z-fighting colors/ textures
// * add texture and white default texture to batch rendering cube.
// * generate terrain?
// * stencil test outline object
// * compile and read shader for presetting uniform? the cherno
// * load models assimp?
// * batch rendering models?
// * memory profiler
// * light system PBR?

int main(int argc, char *argv[])
{
    engine *Engine = EngineConstructAndInit(1280, 960, DEBUG_MODE);

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;
    float waitTime = 1.0f;

    PrepareEmbededAxisDebugRendering(Engine->Renderer);
    PrepareCubeBatchRendering(Engine->Renderer);

    while (Engine->GlobalState == ENGINE_ACTIVE)
    {
	float currentFrame = (float)glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;

	EnginePollEvents(Engine);
	if (Engine->InputState->Keyboard[GLFW_KEY_ESCAPE])
	    Engine->GlobalState = ENGINE_TERMINATE;
	if (Engine->InputState->Keyboard[GLFW_KEY_W])
	    ProcessCameraKeyboard(Engine->Camera, FORWARD, deltaTime);
	if (Engine->InputState->Keyboard[GLFW_KEY_S])
	    ProcessCameraKeyboard(Engine->Camera, BACKWARD, deltaTime);
	if (Engine->InputState->Keyboard[GLFW_KEY_A])
	    ProcessCameraKeyboard(Engine->Camera, LEFT, deltaTime);
	if (Engine->InputState->Keyboard[GLFW_KEY_D])
	    ProcessCameraKeyboard(Engine->Camera, RIGHT, deltaTime);	
	if (Engine->InputState->Keyboard[GLFW_KEY_SPACE])
	    ProcessCameraKeyboard(Engine->Camera, UP, deltaTime);	
	if (Engine->InputState->Keyboard[GLFW_KEY_LEFT_CONTROL])
	    ProcessCameraKeyboard(Engine->Camera, DOWN, deltaTime);	
	if (Engine->InputState->MouseLeftButton)
	{
	    UpdateMouseOffset(Engine->InputState);
	    ProcessCameraMouseMovement(Engine->Camera,
				       Engine->InputState->MouseOffsetX,
				       Engine->InputState->MouseOffsetY);
	}

	ResetRendererStats(Engine->Renderer);
        StartRendering(Engine->Renderer->Shader, Engine->Camera);	

        StartNewCubeBatch(Engine->Renderer);
	float mapSize = 10.0f;
	float r = 0.0f;
	float g = 0.0f;
	float b = 1.0f;
	float posX = 0.0f;
	for (float i = 0.0f; i < mapSize; i += 1.0f)
	{
	    float posZ = -2.0f;
	    for (int y = 0; y < mapSize; y++)
	    {
		if (b == 1.0f)
		{
		    r = 1.0f;
		    g = 0.0f;
		    b = 0.0f;
		} else if (r == 1.0f)
		{
		    r = 0.0f;
		    g = 1.0f;
		    b = 0.0f;
		} else if (g == 1.0f)
		{
		    r = 0.0f;
		    g = 0.0f;
		    b = 1.0f;
		}
		    
		AddCubeToBuffer(
		    Engine->Renderer,
		    { posX, 0.0f, posZ },
		    { 2.0f, 1.0f, 2.0f },
		    { r, g, b, 1.0f });
		posZ -= 2.0f;
	    }
	    posX += 2.0f;
	}

	// float r2 = 0.12f;
	// float b2 = 0.25f;
	// float pos2 = 2.0f;
	// for (float i = 0.0f; i < 100.0f; i += 1.0f)
	// {
	//     AddCubeToBuffer(
	// 	Engine->Renderer,
	// 	{ pos2, 0.0f, 0.0f },
	// 	{ 2.0f, 2.0f, 2.0f },
	// 	{ r2, 0.0f, b2, 1.0f });

	//     r2 += 0.05f;
	//     b2 += 0.05f;
	//     pos2 += 2.0f;
	// }
	
        CloseCubeBatch(Engine->Renderer);
        FlushCubeBatch(Engine->Renderer);

	if (Engine->DebugMode)
	{
	    DrawDebug(Engine->Renderer);	
	    DrawDebugOverlay(Engine->Renderer->Stats, deltaTime);
	}

        SwapBufferAndFinish(Engine->Window);
    }

    DeleteEngine(Engine);
    return 0;
}
