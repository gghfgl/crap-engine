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

static bool activeWindow = false;
static int MAPSIZE = 10;
BYTE bArray[4];
int iResult = 0;
// entity_cube selectedObject;
int selectedID = 0;

int main(int argc, char *argv[])
{
    engine *Engine = EngineConstructAndInit(1280, 960, DEBUG_MODE | VSYNC);
    PrepareEmbededAxisDebugRendering(Engine->Renderer);
    PrepareCubeBatchRendering(Engine->Renderer);

    CreateTestContainers();

    while (Engine->GlobalState == ENGINE_ACTIVE)
    {
	// color picking
	int invertMouseY = Engine->Height - (int)Engine->InputState->MousePosY;
	glReadPixels((int)Engine->InputState->MousePosX, invertMouseY,
		     1, 1, GL_RGB, GL_UNSIGNED_BYTE, bArray);
	iResult = GetIndexByColor(bArray[0], bArray[1], bArray[2]);

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
	if (Engine->InputState->MouseLeftButton && !activeWindow)
	{
	    UpdateMouseOffset(Engine->InputState);
	    ProcessCameraMouseMovement(Engine->Camera,
				       Engine->InputState->MouseOffsetX,
				       Engine->InputState->MouseOffsetY);

	    if (CONTAINER_ENTITIES.find(iResult) != CONTAINER_ENTITIES.end())
		selectedID = CONTAINER_ENTITIES[iResult].ID;
	    else
	        selectedID = 0;

	}

	ResetRendererStats(Engine->Renderer);
        StartRendering(Engine->Renderer, Engine->Camera);

	// NOTE: CLASSIC RENDERING ======================================
	// debug tools
	if (Engine->DebugMode)
	    DrawAxisDebug(Engine->Renderer);	

	DrawTerrain(Engine->Renderer, MAPSIZE);

	// NOTE: STENCIL RENDERING ======================================
	// TODO: should be under iResult condition?
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilMask(0xFF);

	StartNewBatchCube(Engine->Renderer);
	for (std::pair<int, entity_cube> element : CONTAINER_ENTITIES)
	{
	    DrawCubeContainer(Engine->Renderer, element.second, 1.0f);
	}
	CloseBatchCube(Engine->Renderer);
	FlushBatchCube(Engine->Renderer);

	if (CONTAINER_ENTITIES.find(iResult) != CONTAINER_ENTITIES.end() || selectedID != 0)
	{
	    StartStencilRendering(Engine->Renderer, Engine->Camera);
	    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	    glStencilMask(0x00);
	    glDisable(GL_DEPTH_TEST);

	    StartNewBatchCube(Engine->Renderer);
	    DrawCubeContainer(Engine->Renderer, CONTAINER_ENTITIES[selectedID != 0 ? selectedID : iResult], 1.1f);
	    CloseBatchCube(Engine->Renderer);
	    FlushBatchCube(Engine->Renderer);

	    StopRenderingStencil();	    
	}

	// NOTE: UI RENDERING ======================================
	StartImGuiRendering();	
	// STATS OVERLAY
	DrawDebugOverlay(Engine);

	// TERRAIN TOOL
	ImGui::SetNextWindowPos(ImVec2(10, 160));
	ImGui::SetNextWindowSize(ImVec2(200, 220));
	ImGui::Begin("data", nullptr, ImGuiWindowFlags_NoResize);
	ImGui::Text("Terrain:");
	ImGui::Separator();
	ImGui::SliderInt("range", &MAPSIZE, 0, 100);
	ImGui::Separator();
	ImGui::Text("Picking:");
	ImGui::Separator();
	ImGui::Text("mX= %d / mYinvert %d", (int)Engine->InputState->MousePosX, invertMouseY);
	ImGui::Text("r=%d g=%d b=%d", bArray[0], bArray[1], bArray[2]);
	ImGui::Text("bitValue= %d", iResult);
	if (ImGui::IsWindowFocused())
	    activeWindow = true;
	else
	    activeWindow = false;
	ImGui::End();

	// OBJECT PANEL
	ImGui::SetNextWindowPos(ImVec2(10, 390));
	ImGui::SetNextWindowSize(ImVec2(420, 300));
	ImGui::Begin("Objects", nullptr, ImGuiWindowFlags_NoResize); // TODO ImGuiWindowFlags_NoResize
	// left
	static int selected = 0;
	ImGui::BeginChild("left pane", ImVec2(150, 0), true);

	for (std::pair<int, entity_cube> element : CONTAINER_ENTITIES)	 
	{
	    char label[128];
	    sprintf_s(label, "obj %d", element.first);
	    if (ImGui::Selectable(label, selectedID == element.first))
		selectedID = element.first;
	}

	ImGui::EndChild();
	ImGui::SameLine();
	// right
	ImGui::BeginGroup();
	ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing())); // Leave room for 1 line below us
	ImGui::Text("obj: %d", selected);
	ImGui::Separator();
	if (selectedID != 0)
	{
	    ImGui::Text("ID: %d", CONTAINER_ENTITIES[selectedID].ID);
	    ImGui::Text("mem: %p", &CONTAINER_ENTITIES[selectedID]);
	    ImGui::Text("State: %s",
			(CONTAINER_ENTITIES[selectedID].State == ENTITY_STATIC ? "STATIC" : "DYNAMIC"));
	    ImGui::Text("Pos x=%.2f y=%.2f z=%.2f",
			CONTAINER_ENTITIES[selectedID].Position.x,
			CONTAINER_ENTITIES[selectedID].Position.y,
			CONTAINER_ENTITIES[selectedID].Position.z);

	    ImGui::Text("Size x=%.2f y=%.2f z=%.2f w=%.2f",
			CONTAINER_ENTITIES[selectedID].Size.x,
			CONTAINER_ENTITIES[selectedID].Size.y,
			CONTAINER_ENTITIES[selectedID].Size.z,
			CONTAINER_ENTITIES[selectedID].Color.w);

	    ImGui::Text("Color r=%.2f g=%.2f b=%.2f a=%.2f",
			CONTAINER_ENTITIES[selectedID].Color.r,
			CONTAINER_ENTITIES[selectedID].Color.g,
			CONTAINER_ENTITIES[selectedID].Color.b,
			CONTAINER_ENTITIES[selectedID].Color.a);
	}
	ImGui::EndChild();
	ImGui::EndGroup();
	if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows))
	    activeWindow = true;
	else if (!activeWindow)
	    activeWindow = false;
	ImGui::End();

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
    AddCubeToBuffer(Renderer,
		    container.Position,
		    { container.Size.x, container.Size.y, container.Size.z, scale },
		    container.Color);
}

void DrawCubeContainers(renderer *Renderer, std::unordered_map<int, entity_cube> containers, float scale)
{
    for (std::pair<int, entity_cube> element : containers)
	AddCubeToBuffer(Renderer,
			element.second.Position,
			{ element.second.Size.x, element.second.Size.y, element.second.Size.z, scale },
			element.second.Color);
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
