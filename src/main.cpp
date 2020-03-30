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
// * implement game entity and level?
// * abstract UI debug
// * move object from slot to slot
// * set of predef camera position
// * add click action to container object and open imgui inventory?
// * move object between inventories
// * lock camera movement
// * implement frame rate counter ms VSYNC ?
// * logger ?
// * in fuction time profiler (handemade hero fast thread id retrieval)
// * work on z-buffer to avoid z-fighting colors/ textures
// * add texture and white default texture to batch rendering cube.
// * generate terrain (advanced)?
// * compile and read shader for presetting uniform? the cherno
// * load models assimp?
// * batch rendering models?
// * memory profiler
// * light system PBR?

glm::vec4 GetColorByIndex(int index);
void CrapColors(float *r, float *g, float *b);
void DrawTerrain(renderer *Renderer, int mapSize);
void DrawCubeContainer(renderer *Renderer, entity_cube container, float scale);
void DrawCubeContainers(renderer *Renderer, std::unordered_map<int,entity_cube> containers, float scale);
void CreateTestContainers();

#define RGB_WHITE (0xFF | (0xFF<<8) | (0xFF<<16))
//std::vector<entity_cube> Containers; // TODO: NEXT STEP HERE!!!!! Fill a cube vector 
std::unordered_map<int, entity_cube> CONTAINER_ENTITIES;

bool noWindowFocus = true;
static int MAPSIZE = 10;
BYTE bArray[4];
int iResult = 0;

int main(int argc, char *argv[])
{
    engine *Engine = EngineConstructAndInit(1280, 960, DEBUG_MODE | VSYNC);
    PrepareEmbededAxisDebugRendering(Engine->Renderer);
    PrepareCubeBatchRendering(Engine->Renderer);

    CreateTestContainers();

    while (Engine->GlobalState == ENGINE_ACTIVE)
    {
	UpdateDeltaTimeAndFPS(Engine->Time);
	EnginePollEvents(Engine);
	if (Engine->InputState->Keyboard[GLFW_KEY_ESCAPE])
	    Engine->GlobalState = ENGINE_TERMINATE;
	if (Engine->InputState->Keyboard[GLFW_KEY_W])
	    ProcessCameraKeyboard(Engine->Camera, FORWARD, (float)Engine->Time->DeltaTime);
	if (Engine->InputState->Keyboard[GLFW_KEY_S])
	    ProcessCameraKeyboard(Engine->Camera, BACKWARD, (float)Engine->Time->DeltaTime);
	if (Engine->InputState->Keyboard[GLFW_KEY_A])
	    ProcessCameraKeyboard(Engine->Camera, LEFT, (float)Engine->Time->DeltaTime);
	if (Engine->InputState->Keyboard[GLFW_KEY_D])
	    ProcessCameraKeyboard(Engine->Camera, RIGHT, (float)Engine->Time->DeltaTime);	
	if (Engine->InputState->Keyboard[GLFW_KEY_SPACE])
	    ProcessCameraKeyboard(Engine->Camera, UP, (float)Engine->Time->DeltaTime);	
	if (Engine->InputState->Keyboard[GLFW_KEY_LEFT_CONTROL])
	    ProcessCameraKeyboard(Engine->Camera, DOWN, (float)Engine->Time->DeltaTime);	
	if (Engine->InputState->MouseLeftButton && noWindowFocus)
	{
	    UpdateMouseOffset(Engine->InputState);
	    ProcessCameraMouseMovement(Engine->Camera,
				       Engine->InputState->MouseOffsetX,
				       Engine->InputState->MouseOffsetY);
	}

	ResetRendererStats(Engine->Renderer);
        StartRendering(Engine->Renderer, Engine->Camera);
	int invertMouseY = Engine->Height - (int)Engine->InputState->MousePosY;

	// NOTE: CLASSIC RENDERING ======================================
	// debug tools
	if (Engine->DebugMode)
	    DrawAxisDebug(Engine->Renderer);	

	DrawTerrain(Engine->Renderer, MAPSIZE);

	// NOTE: STENCIL RENDERING ======================================
	// TODO: should be under iResult condition?
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilMask(0xFF);

	DrawCubeContainers(Engine->Renderer, CONTAINER_ENTITIES, 1.0f);

	// color picking
	glReadPixels((int)Engine->InputState->MousePosX, invertMouseY,
		     1, 1, GL_RGB, GL_UNSIGNED_BYTE, bArray);
	iResult = GetIndexByColor(bArray[0], bArray[1], bArray[2]);

	unsigned int selectedID = 0;
	if (CONTAINER_ENTITIES.find(iResult) != CONTAINER_ENTITIES.end())
	{	    
	    StartStencilRendering(Engine->Renderer, Engine->Camera);
	    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	    glStencilMask(0x00);
	    glDisable(GL_DEPTH_TEST);

	    std::cout << "picking " << CONTAINER_ENTITIES[iResult].ID << std::endl; 
	    DrawCubeContainer(Engine->Renderer, CONTAINER_ENTITIES[iResult], 1.1f);
	    selectedID = CONTAINER_ENTITIES[iResult].ID;

	    StopRenderingStencil();
	}

	// NOTE: UI RENDERING ======================================
	StartImGuiRendering();
	// terrain tool generator
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
	ImGui::Text("selectedID= %d", selectedID);
	ImGui::End();
	// stats overlay
	DrawDebugOverlay(Engine);

	RenderImGui();

	// NOTE: SWAP BUFFER ======================================
	SwapBufferAndFinish(Engine->Window);
    }

    DeleteEngine(Engine);
    return 0;
}

void CreateTestContainers()
{
    entity_cube containerOne = CreateEntityCube({ 3.0f, 0.5f, 0.0f },
					     { 1.0f, 1.0f, 1.0f, 1.0f },
					     { 1.0f, 1.0f, 1.0f, 1.0f });
    entity_cube containerTwo = CreateEntityCube({ -3.0f, 0.5f, 0.0f },
					     { 1.0f, 1.0f, 1.0f, 1.0f },
					     { 0.5f, 1.0f, 1.0f, 1.0f });
    
    if (CONTAINER_ENTITIES.find(containerOne.ID) == CONTAINER_ENTITIES.end())
	CONTAINER_ENTITIES[containerOne.ID] = containerOne;
    if (CONTAINER_ENTITIES.find(containerTwo.ID) == CONTAINER_ENTITIES.end())
	CONTAINER_ENTITIES[containerTwo.ID] = containerTwo;
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

void DrawCubeContainer(renderer *Renderer, entity_cube container, float scale)
{
    StartNewBatchCube(Renderer);
    AddCubeToBuffer(Renderer,
		    container.Position,
		    { container.Size.x, container.Size.y, container.Size.z, scale },
		    container.Color);
    CloseBatchCube(Renderer);
    FlushBatchCube(Renderer);
}

void DrawCubeContainers(renderer *Renderer, std::unordered_map<int, entity_cube> containers, float scale)
{
    StartNewBatchCube(Renderer);
    for (std::pair<int, entity_cube> element : containers)
	AddCubeToBuffer(Renderer,
			element.second.Position,
			{ element.second.Size.x, element.second.Size.y, element.second.Size.z, scale },
			element.second.Color);
    CloseBatchCube(Renderer);
    FlushBatchCube(Renderer);
}

void DrawTerrain(renderer *Renderer, int mapSize)
{
    StartNewBatchCube(Renderer);
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
		Renderer,
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
		Renderer,
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
		Renderer,
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
		Renderer,
		{ posX, 0.0f, posZ },
		{ size, 0.5f, size, 1.0f },
		{ r, g, b, 1.0f });
	    posZ += size;
	}
	posX -= size;
    }
    CloseBatchCube(Renderer);
    FlushBatchCube(Renderer);
}
