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
// * implement frame rate counter ms
// * implement game entity and level?
// * in fuction time profiler (handemade hero fast thread id retrieval)
// * work on z-buffer to avoid z-fighting colors/ textures
// * add texture and white default texture to batch rendering cube.
// * generate terrain?
// * stencil test outline object
// * compile and read shader for presetting uniform? the cherno
// * implement logger? check the cherno
// * add batch model renderer?
// * memory profiler
// * light system PBR?

int main(int argc, char *argv[])
{
    // Initialize engine
    engine *Engine = EngineConstruct();
    InitEngine(Engine, 1280, 960, NO_DEBUG);

    PrepareDebugRendering(Engine->Renderer);
    PrepareCubeBatchRendering(Engine->Renderer);

    // needed for enforcing 60fps
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;
    float waitTime = 1.0f;

    while (Engine->GlobalState == ENGINE_ACTIVE)
    {
	//float currentFrame = (float)glfwGetTime();
        EngineUpdate(Engine, deltaTime);

	// DeltaTime TODO build a function to auto retrieve this?
	float currentFrame = (float)glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;

	if (Engine->InputState->Keyboard[GLFW_KEY_ESCAPE])
	{
	    Engine->GlobalState = ENGINE_TERMINATE;
	}

	if (Engine->InputState->Keyboard[GLFW_KEY_W])
	{
	    ProcessCameraKeyboard(Engine->Camera, FORWARD, deltaTime);
	}

	if (Engine->InputState->Keyboard[GLFW_KEY_S])
	{
	    ProcessCameraKeyboard(Engine->Camera, BACKWARD, deltaTime);
	}

	if (Engine->InputState->Keyboard[GLFW_KEY_A])
	{
	    ProcessCameraKeyboard(Engine->Camera, LEFT, deltaTime);
	}

	if (Engine->InputState->Keyboard[GLFW_KEY_D])
	{
	    ProcessCameraKeyboard(Engine->Camera, RIGHT, deltaTime);
	}
	
	if (Engine->InputState->MouseLeftButton)
	{
	    UpdateMouseOffset(Engine->InputState);
	    ProcessCameraMouseMovement(Engine->Camera, Engine->InputState->MouseOffsetX, Engine->InputState->MouseOffsetY);
	}

	// Render
        StartRendering(Engine);
	
        ResetRendererStats(Engine->Renderer); // TODO: put in start rendering?
        DrawDebug(Engine->Renderer);	
        StartNewCubeBatch(Engine->Renderer);

	float r = 0.12f;
	float g = 0.25f;
	float pos = 2.0f;
	for (float i = 0.0f; i < 1000.0f; i += 1.0f)
	{
	    AddCubeToBuffer(
		Engine->Renderer,
		{ 0.0f, 0.0f, -pos },
		{ 2.0f, 2.0f, 2.0f },
		{ r, g, 0.0f, 1.0f });

	    r += 0.05f;
	    g += 0.05f;
	    pos += 2.0f;
	}

	float r2 = 0.12f;
	float b2 = 0.25f;
	float pos2 = 2.0f;
	for (float i = 0.0f; i < 100.0f; i += 1.0f)
	{
	    AddCubeToBuffer(
		Engine->Renderer,
		{ pos2, 0.0f, 0.0f },
		{ 2.0f, 2.0f, 2.0f },
		{ r2, 0.0f, b2, 1.0f });

	    r2 += 0.05f;
	    b2 += 0.05f;
	    pos2 += 2.0f;
	}

	
        CloseCubeBatch(Engine->Renderer);
        FlushCubeBatch(Engine->Renderer);

	if (Engine->OverlayState == OVERLAY_DEBUG)
	{
	    DisplayDebugOverlay(Engine, waitTime);
	}

        StopRendering(Engine);

	// currentFrame = (float)glfwGetTime() - currentFrame;
	// while (waitTime > 0)
	// {
	//     float frame = (float)glfwGetTime() - currentFrame;
	//     waitTime = 16.0f - frame;
	//     std::cout << waitTime << std::endl;
	// }

	// waitTime = 16.0f;
	// deltaTime = 33.0f;
    }

    // Delete
    DeleteEngine(Engine);

    return 0;
}
