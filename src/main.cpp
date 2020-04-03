#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>


#include "plateform.h"

/* NOTE: should implement
   - depth test
   - face culling
   - MSAA anti aliasing
   - frame rate counter ms /  VSYNC viz
   - stencil test outline object + picking object by color (GPU)
   - quick abstract UI debug
   - stencil test outline object + picking object by ray casting (CPU)
   - draw sphere ...
*/

/* TODO:
   - fit shader version with gl version ?
   - draw ray casting debug sphere ? or next
   - add Ray casting OBB test for cubes
   - move object from slot to slot
   - trace line from mouse to objects / terrain slots?
   - read level design from file
   - set of predef camera position
   - add click action to container object and open imgui inventory?
   - move object between inventories
   - lock camera movement from to terrain space
   - z-fighting
   - logger ?
   - in fuction time profiler (handemade hero fast thread id retrieval)
   - work on z-buffer to avoid z-fighting colors/ textures
   - add texture and white default texture to batch rendering cube.
   - generate terrain (advanced)?
   - compile and read shader for presetting uniform? the cherno
   - load models assimp?
   - batch rendering models?
   - memory profiler
   - light system PBR?
   - blending
   - framebuffer ?
   - mipmap ?
   - cubemap / skybox / reflect ?
   - geometry shader ?
   - instancing ?
*/
	
static bool  globalActiveWindow        = false;
static int   globalSliderMapSize       = 130;
unsigned int globalContainerSelectedID = 0;
unsigned int globalContainerHoveredID  = 0;
unsigned int globalTerrainHoveredID    = 0;

std::unordered_map<int, entity_cube> GAME_TERRAIN_ENTITIES;
std::unordered_map<int, entity_cube> GAME_CONTAINER_ENTITIES;
std::unordered_map<int, int> GAME_SLOT_RELATIONSHIPS;

void DrawSettingsPanel(engine *Engine, int &mapSize, std::unordered_map<int, entity_cube> &objects, std::unordered_map<int, int> &slots, bool &focus);

void CreateTestTerrain(int mapSize, std::unordered_map<int, entity_cube> &terrain, std::unordered_map<int, int> &slots);
void CreateTestContainers();
void CreateTestSpheres(float radius, int slacks, int slices); // TODO: entity_sphere

void PushEntityCubeToBuffer(renderer *Renderer, entity_cube container, float scale);
void DrawSpheres();

int main(int argc, char *argv[])
{
    engine *Engine = EngineConstructAndInit(1280, 960, DEBUG_MODE | VSYNC);
    PrepareEmbededAxisDebugRendering(Engine->Renderer);
    PrepareCubeBatchRendering(Engine->Renderer);
    
    CreateTestContainers();
    //CreateTestSpheres(0.5f, 20, 20); // dirty way to create spheres

    while (Engine->GlobalState == ENGINE_ACTIVE)
    {
	CreateTestTerrain(globalSliderMapSize, GAME_TERRAIN_ENTITIES, GAME_SLOT_RELATIONSHIPS);

	// Ray casting test
	glm::vec3 rayWorld = MouseRayDirectionWorld((float)Engine->InputState->MousePosX,
						    (float)Engine->InputState->MousePosY,
						    Engine->Width,
						    Engine->Height,
						    Engine->ProjMatrix,
						    GetCameraViewMatrix(Engine->Camera));

	for (std::pair<int, entity_cube> element : GAME_TERRAIN_ENTITIES)
	{
	    float rayIntersection = 0.0f;
	    glm::vec3 spherePos = glm::vec3(
		element.second.Position.x + element.second.Scale / 2,
		element.second.Position.y + element.second.Scale / 2,
		element.second.Position.z + element.second.Scale / 2);
	    
	    if (RaySphereIntersection(Engine->Camera->Position, rayWorld, spherePos, 0.5f, &rayIntersection))
	    {
		if (element.second.State == ENTITY_STATE_SLOT)
		{
		    globalTerrainHoveredID = element.second.ID;
		    break;
		}
	    }
	    else
		globalTerrainHoveredID = 0;
	}

	for (std::pair<int, entity_cube> element : GAME_CONTAINER_ENTITIES)
	{
	    float rayIntersection = 0.0f;
	    glm::vec3 spherePos = glm::vec3(
		element.second.Position.x + element.second.Scale / 2,
		element.second.Position.y + element.second.Scale / 2,
		element.second.Position.z + element.second.Scale / 2);
	    
	    if (RaySphereIntersection(Engine->Camera->Position, rayWorld, spherePos, 0.5f, &rayIntersection))
	    {
	        globalContainerHoveredID = element.second.ID;
		break;
	    }
	    else
		globalContainerHoveredID = 0;
	}
	
	// I/O
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
	if (Engine->InputState->MouseLeftButton && !globalActiveWindow)
	{
	    UpdateMouseOffset(Engine->InputState);
	    ProcessCameraMouseMovement(Engine->Camera,
				       Engine->InputState->MouseOffsetX,
				       Engine->InputState->MouseOffsetY);

	    if (globalContainerHoveredID != 0)
		globalContainerSelectedID = globalContainerHoveredID;
	    else
	        globalContainerSelectedID = 0;
	}

	// NOTE: START RENDERING ======================================
	ResetRendererStats(Engine->Renderer);
        StartRendering(Engine->Renderer, Engine->Camera);

	// NOTE: CLASSIC RENDERING ======================================
	// debug tools
	if (Engine->DebugMode)
	    DrawAxisDebug(Engine->Renderer);	

	StartNewBatchCube(Engine->Renderer);
	for (std::pair<int, entity_cube> element : GAME_TERRAIN_ENTITIES)
	{
	    if (globalTerrainHoveredID != 0 && globalTerrainHoveredID == element.second.ID)
	    {
		entity_cube hoveredSlot = EntityCubeConstruct(element.second.ID,
							      element.second.Position,
							      { 1.0f, 0.5f, 1.0f },
							      element.second.Scale,
							      { 0.7f, 0.7f, 0.7f, 1.0f },
							      element.second.State);

		PushEntityCubeToBuffer(Engine->Renderer, hoveredSlot, 1.0f);
	    } else
		PushEntityCubeToBuffer(Engine->Renderer, element.second, 1.0f);
	}
	//DrawSpheres();
	CloseBatchCube(Engine->Renderer);
	FlushBatchCube(Engine->Renderer);

	// NOTE: STENCIL RENDERING ======================================
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilMask(0xFF);

	StartNewBatchCube(Engine->Renderer);
	for (std::pair<int, entity_cube> element : GAME_CONTAINER_ENTITIES)
	{
	    PushEntityCubeToBuffer(Engine->Renderer, element.second, 1.0f);
	}
	CloseBatchCube(Engine->Renderer);
	FlushBatchCube(Engine->Renderer);

	if (globalContainerHoveredID != 0 || globalContainerSelectedID != 0)
	{
	    StartStencilRendering(Engine->Renderer, Engine->Camera);

	    StartNewBatchCube(Engine->Renderer);
	    if (globalContainerSelectedID != 0)
		PushEntityCubeToBuffer(Engine->Renderer, GAME_CONTAINER_ENTITIES[globalContainerSelectedID], 1.1f);
	    if (globalContainerHoveredID != 0)
		PushEntityCubeToBuffer(Engine->Renderer, GAME_CONTAINER_ENTITIES[globalContainerHoveredID], 1.1f);
	    CloseBatchCube(Engine->Renderer);
	    FlushBatchCube(Engine->Renderer);

	    StopRenderingStencil();	    
	}
	
	// NOTE: UI RENDERING ======================================
	StartImGuiRendering();	

	DrawDebugOverlay(Engine);
        DrawSettingsPanel(Engine,
			  globalSliderMapSize,
			  GAME_CONTAINER_ENTITIES,
			  GAME_SLOT_RELATIONSHIPS,
			  globalActiveWindow);	

	RenderImGui();
	
	// NOTE: SWAP BUFFER ======================================
	SwapBufferAndFinish(Engine->Window);
    }

    DeleteEngine(Engine);
    return 0;
}

void CreateTestContainers()
{
    entity_cube containerOne = EntityCubeConstruct(1,
						   { 3.0f, 0.5f, -2.0f },
						   { 1.0f, 1.0f, 1.0f },
						   1.0f,
						   { 1.0f, 0.0f, 0.0f, 1.0f },
						   ENTITY_STATE_DYNAMIC);
    entity_cube containerTwo = EntityCubeConstruct(2,
						   { 5.0f, 0.5f, -4.0f },
						   { 1.0f, 1.0f, 1.0f },
						   1.0f,
						   { 0.0f, 0.0f, 1.0f, 1.0f },
						   ENTITY_STATE_DYNAMIC);
    
    if (GAME_CONTAINER_ENTITIES.find(containerOne.ID) == GAME_CONTAINER_ENTITIES.end())
	GAME_CONTAINER_ENTITIES[containerOne.ID] = containerOne;
    if (GAME_CONTAINER_ENTITIES.find(containerTwo.ID) == GAME_CONTAINER_ENTITIES.end())
	GAME_CONTAINER_ENTITIES[containerTwo.ID] = containerTwo;
}

void CreateTestTerrain(int mapSize, std::unordered_map<int, entity_cube> &terrain, std::unordered_map<int, int> &slots)
{
    terrain.clear();
    slots.clear();
    glm::vec4 color = { 0.2f, 0.2f, 0.2f, 1.0f };
    glm::vec4 colorH = { 0.25f, 0.25f, 0.25f, 1.0f };
    glm::vec3 size = { 1.0f, 0.5f, 1.0f };
    float scale = 1.0f;
    entity_state state = ENTITY_STATE_STATIC;
    float posX = 0.0f;
    bool slot = true;
    unsigned int id = 1;
    int side = (int)std::sqrt(mapSize);
    for (int i = 0; i < side; i++)
    {
	float posZ = -size.z;
	for (int y = 0; y < side; y++)
	{
	    glm::vec4 c = color;
	    entity_state s = state;
	    if (slot && i % 4 == 1)
	    {
		c = colorH;
		s = ENTITY_STATE_SLOT;
		slots[id] = 0;
	    }

	    entity_cube t = EntityCubeConstruct(id, { posX, 0.0f, posZ }, size, scale, c, s);
	    terrain[t.ID] = t;
	    posZ -= size.z;
	    slot = !slot;
	    id++;
	}
	posX += size.x;
    }

    // posX = -1.0f;
    // for (int i = 0; i < mapSize / 2; i++)
    // {
    // 	float posZ = -size.z;
    // 	for (int y = 0; y < mapSize / 2; y++)
    // 	{
    // 	    glm::vec4 c = color;
    // 	    entity_state s = state;
    // 	    if (slot)
    // 	    {
    // 		c = colorH;
    // 		s = ENTITY_STATE_SLOT;
    // 	    }

    // 	    entity_cube t = EntityCubeConstruct(id, { posX, 0.0f, posZ }, size, scale, c, s);
    // 	    terrain[t.ID] = t;
    // 	    posZ -= size.z;
    // 	    slot = !slot;
    // 	    id++;
    // 	}
    // 	posX -= size.x;
    // }

    // slot = false;
    // posX = 0.0f;
    // for (int i = 0; i < mapSize / 2; i++)
    // {
    // 	float posZ = 0.0f;
    // 	for (int y = 0; y < mapSize / 2; y++)
    // 	{
    // 	    glm::vec4 c = color;
    // 	    entity_state s = state;
    // 	    if (slot)
    // 	    {
    // 		c = colorH;
    // 		s = ENTITY_STATE_SLOT;
    // 	    }

    // 	    entity_cube t = EntityCubeConstruct(id, { posX, 0.0f, posZ }, size, scale, c, s);
    // 	    terrain[t.ID] = t;
    // 	    posZ += size.z;
    // 	    slot = !slot;
    // 	    id++;
    // 	}
    // 	posX += size.x;
    // }

    // posX = -1.0f;
    // for (int i = 0; i < mapSize / 2; i++)
    // {
    // 	float posZ = 0.0f;
    // 	for (int y = 0; y < mapSize / 2; y++)
    // 	{
    // 	    glm::vec4 c = color;
    // 	    entity_state s = state;
    // 	    if (slot)
    // 	    {
    // 		c = colorH;
    // 		s = ENTITY_STATE_SLOT;
    // 	    }

    // 	    entity_cube t = EntityCubeConstruct(id, { posX, 0.0f, posZ }, size, scale, c, s);
    // 	    terrain[t.ID] = t;
    // 	    posZ += size.z;
    // 	    slot = !slot;
    // 	    id++;
    // 	}
    // 	posX -= size.x;
    // }
}

void PushEntityCubeToBuffer(renderer *Renderer, entity_cube cube, float scale)
{
    AddCubeToBuffer(Renderer,
		    cube.Position,
		    { cube.Size.x, cube.Size.y, cube.Size.z },
		    cube.Scale * scale,
		    cube.Color);
}

void DrawSettingsPanel(engine *Engine,
		       int &mapSize,
		       std::unordered_map<int, entity_cube> &containers,
		       std::unordered_map<int, int> &slots,
		       bool &focus)
{
    ImGui::SetNextWindowPos(ImVec2(10, 10));
    ImGui::SetNextWindowSize(ImVec2(410, 700));
    ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_NoResize);

    // Engine
    ShowEngineSettingsWindow(Engine);

    // World
    if (ImGui::CollapsingHeader("World settings", ImGuiTreeNodeFlags_DefaultOpen))
    {
	ImGui::Text("slots: %03d/%03d", 0, slots.size());
	ImGui::SliderInt("floor", &mapSize, 0, 400);
	ImGui::Separator();
    }

    // Containers
    if (ImGui::CollapsingHeader("Object settings", ImGuiTreeNodeFlags_DefaultOpen))
    {
    static int selected = 0;
    ImGui::BeginChild("left pane", ImVec2(120, 150));
    for (std::pair<int, entity_cube> element : containers)	 
    {
    	char label[128];
    	sprintf_s(label, "obj %03d", element.first);
    	if (ImGui::Selectable(label, (int)globalContainerSelectedID == element.first))
    	    globalContainerSelectedID = element.first;
    }

    ImGui::EndChild();
    ImGui::SameLine();

    ImGui::BeginChild("right pane", ImVec2(0, 150));
    if (globalContainerSelectedID != 0)
    {
    	ImGui::Text("ID: %d", containers[globalContainerSelectedID].ID);
    	ImGui::Text("mem: %p", &containers[globalContainerSelectedID]);
    	ImGui::Text("State: %s",
    		    (containers[globalContainerSelectedID].State == ENTITY_STATE_STATIC ? "STATIC" : "DYNAMIC"));
    	ImGui::Text("Pos x=%.2f y=%.2f z=%.2f",
    		    containers[globalContainerSelectedID].Position.x,
    		    containers[globalContainerSelectedID].Position.y,
    		    containers[globalContainerSelectedID].Position.z);

    	ImGui::Text("Size x=%.2f y=%.2f z=%.2f w=%.2f",
    		    containers[globalContainerSelectedID].Size.x,
    		    containers[globalContainerSelectedID].Size.y,
    		    containers[globalContainerSelectedID].Size.z,
    		    containers[globalContainerSelectedID].Color.w);

    	ImGui::Text("Color r=%.2f g=%.2f b=%.2f a=%.2f",
    		    containers[globalContainerSelectedID].Color.r,
    		    containers[globalContainerSelectedID].Color.g,
    		    containers[globalContainerSelectedID].Color.b,
    		    containers[globalContainerSelectedID].Color.a);
    }
    ImGui::EndChild();
    ImGui::Separator();
    }

    // Slots
    if (ImGui::CollapsingHeader("Slots settings", ImGuiTreeNodeFlags_DefaultOpen))
    {
	const char* items[] = { "AAAA", "BBBB", "CCCC", "DDDD", "EEEE", "FFFF", "GGGG", "HHHH", "IIII", "JJJJ", "KKKK", "LLLLLLL", "MMMM", "OOOOOOO" };
        const char* item_current = NULL;            // Here our selection is a single pointer stored outside the object.

	ImGui::Columns(2);
	for (std::pair<int, int> sl : slots)
	{
	    char buf1[32];
	    sprintf_s(buf1, "%03d", sl.first);
	    ImGui::Button(buf1, ImVec2(-FLT_MIN, 0.0f));
	    ImGui::NextColumn();

	    // char buf2[32];
	    // sprintf_s(buf2, "%03d", element.second);
	    // ImGui::Button(buf2, ImVec2(-FLT_MIN, 0.0f));
	    // ImGui::NextColumn();

	    if (ImGui::BeginCombo("test", item_current)) // The second parameter is the label previewed before opening the combo.
	    {
		for (std::pair<int, entity_cube> ct : containers)	 
		{
		    ImGui::ColorButton("color", ImVec4(ct.second.Color.r,
						       ct.second.Color.g,
						       ct.second.Color.b,
						       ct.second.Color.a));
		    ImGui::SameLine();
		    bool is_selected = (item_current == items[1]);
		    if (ImGui::Selectable(items[1], is_selected))
			item_current = items[1];
		    if (is_selected)
			ImGui::SetItemDefaultFocus();   // Set the initial focus when opening the combo (scrolling + for keyboard navigation support in the upcoming navigation branch)
		}

		ImGui::EndCombo();
	    }

	    ImGui::NextColumn();
	}

	ImGui::Columns(1);
	ImGui::Separator();
    }

    if (ImGui::IsWindowFocused())
	focus = true;
    else
	focus = false;
    ImGui::End();
}

// TODO: move to entity (entity_sphere...)
unsigned int SphereVAO, SphereVBO, SphereIBO;
std::vector<uint32_t> sphereIndices;
void CreateTestSpheres(float radius, int stacks, int slices)
{
    std::vector<glm::vec3> vertices;
    glGenVertexArrays(1, &SphereVAO);
    glBindVertexArray(SphereVAO);

    // TODO mem overflow?
    for (int i = 0; i <= stacks; ++i){
        GLfloat V   = i / (float) stacks;
        GLfloat phi = V * glm::pi <float> ();
        
        for (int j = 0; j <= slices; ++j){
            GLfloat U = j / (float) slices;
            GLfloat theta = U * (glm::pi <float> () * 2);
            
            // Calc The Vertex Positions
            GLfloat x = cosf (theta) * sinf (phi);
            GLfloat y = cosf (phi);
            GLfloat z = sinf (theta) * sinf (phi);
            vertices.push_back(glm::vec3(x * radius, y * radius, z * radius));
        }
    }
    
    for (int i = 0; i < slices * stacks + slices; ++i){        
        sphereIndices.push_back (i);
        sphereIndices.push_back (i + slices + 1);
        sphereIndices.push_back (i + slices);
        
        sphereIndices.push_back (i + slices + 1);
        sphereIndices.push_back (i);
        sphereIndices.push_back (i + 1);
    }
    
    glGenBuffers(1, &SphereVBO);
    glBindBuffer(GL_ARRAY_BUFFER, SphereVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (const void*)0);

    glGenBuffers(1, &SphereIBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, SphereIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size() * sizeof(unsigned int), &sphereIndices[0], GL_STATIC_DRAW);
}

void DrawSpheres()
{
    glBindVertexArray(SphereVAO);
    glDrawElements(GL_TRIANGLES, (int)sphereIndices.size(), GL_UNSIGNED_INT, NULL);
}
