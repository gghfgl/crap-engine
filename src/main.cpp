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
// * stencil test outline object + picking object by color
// * move object from slot to slot
// * add click action to container object and open imgui inventory?
// * move object between inventories
// * implement frame rate counter ms VSYNC ?
// * logger ?
// * implement game entity and level?
// * in fuction time profiler (handemade hero fast thread id retrieval)
// * work on z-buffer to avoid z-fighting colors/ textures
// * add texture and white default texture to batch rendering cube.
// * generate terrain (advanced)?
// * compile and read shader for presetting uniform? the cherno
// * load models assimp?
// * batch rendering models?
// * memory profiler
// * light system PBR?

int GetIndexByColor(int r, int g, int b);
glm::vec4 GetColorByIndex(int index);
void CrapColors(float *r, float *g, float *b);
void DrawTerrain(engine *Engine, int mapSize);
void DrawContainer(engine *Engine, float scale);

#define RGB_WHITE (0xFF | (0xFF<<8) | (0xFF<<16))

float deltaTime = 0.0f;
float lastFrame = 0.0f;
float waitTime = 1.0f;
bool noWindowFocus = true;
static int MAPSIZE = 10;
int iResult = 0;

int main(int argc, char *argv[])
{
    engine *Engine = EngineConstructAndInit(1280, 960, DEBUG_MODE);
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
	if (Engine->InputState->MouseLeftButton && noWindowFocus)
	{
	    UpdateMouseOffset(Engine->InputState);
	    ProcessCameraMouseMovement(Engine->Camera,
				       Engine->InputState->MouseOffsetX,
				       Engine->InputState->MouseOffsetY);
	}

	ResetRendererStats(Engine->Renderer);
        StartRendering(Engine->Renderer, Engine->Camera);

	BYTE bArray[4];
	int invertMouseY = 960 - (int)Engine->InputState->MousePosY;
	// NOTE: Draw terrain with slider window
	ImGui::SetNextWindowPos(ImVec2(10, 150));
	ImGui::SetNextWindowSize(ImVec2(200, 220));
	ImGui::Begin("data", nullptr, ImGuiWindowFlags_NoResize);
	noWindowFocus = !ImGui::IsWindowFocused();
	ImGui::Text("Terrain:");
	ImGui::Separator();
	ImGui::SliderInt("range", &MAPSIZE, 0, 100);
	ImGui::Separator();
	ImGui::Text("Picking:");
	ImGui::Separator();
	ImGui::Text("mX= %d /mYinvert %d", (int)Engine->InputState->MousePosX, invertMouseY);
	ImGui::Text("r=%d g=%d b=%d", bArray[0], bArray[1], bArray[2]);
	ImGui::Text("bitValue= %d", iResult);
	ImGui::End();

	DrawTerrain(Engine, MAPSIZE);

	if (Engine->DebugMode)
	{
	    DrawAxisDebug(Engine->Renderer);	
	    DrawDebugOverlay(Engine->Renderer->Stats, deltaTime);
	}	    
	RenderImGui();


	{ // NOTE: Stencil rendering
	    glStencilFunc(GL_ALWAYS, 1, 0xFF);
	    glStencilMask(0xFF);

	    DrawContainer(Engine, 1.0f);

	    // TODO: picking right color
	    glReadPixels((int)Engine->InputState->MousePosX, invertMouseY,
			 1, 1, GL_RGB, GL_UNSIGNED_BYTE, bArray);
	    iResult = GetIndexByColor(bArray[0], bArray[1], bArray[2]);
	    if(iResult == RGB_WHITE)
	    {	    
		StartRenderingStencil(Engine->Renderer, Engine->Camera);
		glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
		glStencilMask(0x00);
		glDisable(GL_DEPTH_TEST);

		DrawContainer(Engine, 1.1f);
	    }

	    StopRenderingStencil();
	}
	
	SwapBufferAndFinish(Engine->Window);
    }

    DeleteEngine(Engine);
    return 0;
}

int GetIndexByColor(int r, int g, int b)
{
   return (r)|(g<<8)|(b<<16);
}

glm::vec4 GetColorByIndex(int index)
{
   int r = index&0xFF;
   int g = (index>>8)&0xFF;
   int b = (index>>16)&0xFF;

   return glm::vec4(float(r)/255.0f, float(g)/255.0f, float(b)/255.0f, 1.0f);
}

void CrapColors(float *r, float *g, float *b) {
    if (*b == 1.0f)
    {
        *r = 1.0f;
        *g = 0.0f;
        *b = 0.0f;
    } else if (*r == 1.0f)
    {
	*r = 0.0f;
	*g = 1.0f;
	*b = 0.0f;
    } else if (*g == 1.0f)
    {
	*r = 0.0f;
	*g = 0.0f;
	*b = 1.0f;
    }
}

void DrawContainer(engine *Engine, float scale)
{
    StartNewBatchCube(Engine->Renderer);
    AddCubeToBuffer(
	Engine->Renderer,
	{ 3.0f, 0.5f, 0.0f },
	{ 1.0f, 1.0f, 1.0f, scale },
	{ 1.0f, 1.0f, 1.0f, 1.0f });
    CloseBatchCube(Engine->Renderer);
    FlushBatchCube(Engine->Renderer);
}

void DrawTerrain(engine *Engine, int mapSize)
{
        StartNewBatchCube(Engine->Renderer);
        float r = 0.0f;
        float g = 0.0f;
        float b = 1.0f;
	float size = 1.0f;
	float posX = 0.0f;
	for (int i = 0; i < mapSize / 2; i++)
	{
	    float posZ = -size;
	    for (int y = 0; y < mapSize / 2; y++)
	    {
		CrapColors(&r, &g, &b);
		AddCubeToBuffer(
		    Engine->Renderer,
		    { posX, 0.0f, posZ },
		    { size, 0.5f, size, 1.0f },
		    { r, g, b, 1.0f });
		posZ -= size;
	    }
	    posX += size;
	}

        posX = -1.0f;
	for (int i = 0; i < mapSize / 2; i++)
	{
	    float posZ = -size;
	    for (int y = 0; y < mapSize / 2; y++)
	    {
		CrapColors(&r, &g, &b);
		AddCubeToBuffer(
		    Engine->Renderer,
		    { posX, 0.0f, posZ },
		    { size, 0.5f, size, 1.0f },
		    { r, g, b, 1.0f });
		posZ -= size;
	    }
	    posX -= size;
	}

	posX = 0.0f;
	for (int i = 0; i < mapSize / 2; i++)
	{
	    float posZ = 0.0f;
	    for (int y = 0; y < mapSize / 2; y++)
	    {
		CrapColors(&r, &g, &b);
		AddCubeToBuffer(
		    Engine->Renderer,
		    { posX, 0.0f, posZ },
		    { size, 0.5f, size, 1.0f },
		    { r, g, b, 1.0f });
		posZ += size;
	    }
	    posX += size;
	}

	posX = -1.0f;
	for (int i = 0; i < mapSize / 2; i++)
	{
	    float posZ = 0.0f;
	    for (int y = 0; y < mapSize / 2; y++)
	    {
		CrapColors(&r, &g, &b);
		AddCubeToBuffer(
		    Engine->Renderer,
		    { posX, 0.0f, posZ },
		    { size, 0.5f, size, 1.0f },
		    { r, g, b, 1.0f });
		posZ += size;
	    }
	    posX -= size;
	}
	
        CloseBatchCube(Engine->Renderer);
        FlushBatchCube(Engine->Renderer);
}
