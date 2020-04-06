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
   - clean renderer construct with shader array?
   - fill engine boolean for Imgui settings
   - stencil outlie probleme z buffer
   - draw ray casting debug sphere see TODO in renderer
   - add Ray casting OBB test with debug bounding box draw
   - position objects from slot attruibution array
   - add texture and white default texture to batch rendering cube.

   - set of predef camera position
   - lock/unlock camera movement from to terrain space
   - load models assimp?
   - read level design from file

   - add click action to container object and open imgui inventory?
   - in fuction time profiler (handemade hero fast thread id retrieval)
   - work on z-buffer to avoid z-fighting colors/ textures
   - move object between inventories
   - z-fighting
   - logger ?
   - put engine settings in file with update and reload program?
   - generate terrain (advanced)?
   - compile and read shader for presetting uniform? the cherno
   - batch rendering models?
   - memory profiler
   - light system PBR?
   - blending
   - framebuffer ?
   - mipmap ?
   - cubemap / skybox / reflect ?
   - geometry shader ?
   - instancing ?
   - learn about compiler (unit, etc ...)
*/
const uint32 globalWidth = 1280;
const uint32 globalHeight = 960;

static bool  globalActiveWindow        = false;
static uint32   globalSliderMapSize       = 130;
uint32 globalContainerSelectedID = 0;
uint32 globalContainerHoveredID  = 0;
uint32 globalTerrainHoveredID    = 0;

glm::vec3 globalCameraStartPosition = glm::vec3(0.0f, 5.0f, 10.0f);

std::unordered_map<uint32, entity_cube> GAME_TERRAIN_ENTITIES;
std::unordered_map<uint32, entity_cube> GAME_CONTAINER_ENTITIES;
std::unordered_map<uint32, uint32> GAME_SLOT_RELATIONSHIPS;

void DrawSettingsPanel(engine *Engine, uint32 width, uint32 height, input_state *InputState, camera *Camera, renderer *Renderr, uint32 &mapSize, std::unordered_map<uint32, entity_cube> &objects, std::unordered_map<uint32, uint32> &slots, bool &focus);
void AttribContainerToSlot(std::unordered_map<uint32, uint32> &slots, std::unordered_map<uint32, entity_cube> &containers, uint32 selectedSlot, uint32 selectedItem);

void CreateTestTerrain(uint32 mapSize, std::unordered_map<uint32, entity_cube> &terrain, std::unordered_map<uint32, uint32> &slots);
void CreateTestContainers();
void CreateTestSpheres(float radius, int slacks, int slices); // TODO: entity_sphere

void PushEntityCubeToBuffer(renderer *Renderer, entity_cube container, float32 scale);
void DrawSpheres();

int main(int argc, char *argv[])
{
    // Init all systems
    engine *Engine = EngineConstruct(globalWidth, globalHeight, DEBUG_MODE | VSYNC);
    camera *Camera = CameraConstruct((float32)globalWidth, (float32)globalHeight, globalCameraStartPosition);
    input_state *InputState = InputStateConstruct(Engine->Window);

    CrapShortcutConstructCompileShaders(Camera->ProjectionMatrix); // TODO: crap
    renderer *Renderer = RendererConstruct(ShaderGetFromStorage("default"),
					   ShaderGetFromStorage("stencil")); // TODO: crap
    
    RendererPrepareDebugAxis(Renderer);
    RendererPrepareCubeBatching(Renderer);
    
    CreateTestContainers();
    //CreateTestSpheres(0.5f, 20, 20); // dirty way to create spheres

    while (Engine->GlobalState == ENGINE_ACTIVE)
    {	
	uint32 WIDTH = 0;
	uint32 HEIGHT = 0;
	EngineGetWindowSize(Engine, WIDTH, HEIGHT);

	CreateTestTerrain(globalSliderMapSize, GAME_TERRAIN_ENTITIES, GAME_SLOT_RELATIONSHIPS);	
	// Ray casting test
	glm::vec3 rayWorld = MouseRayDirectionWorld((float32)InputState->MousePosX,
						    (float32)InputState->MousePosY,
						    WIDTH,
						    HEIGHT,
						    Camera->ProjectionMatrix,
						    CameraGetViewMatrix(Camera));

	for (std::pair<uint32, entity_cube> element : GAME_TERRAIN_ENTITIES)
	{
	    float32 rayIntersection = 0.0f;
	    glm::vec3 spherePos = glm::vec3(
		element.second.Position.x + element.second.Scale / 2,
		element.second.Position.y + element.second.Scale / 2,
		element.second.Position.z + element.second.Scale / 2);
	    
	    if (RaySphereIntersection(Camera->Position, rayWorld, spherePos, 0.5f, &rayIntersection))
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

	for (std::pair<uint32, entity_cube> element : GAME_CONTAINER_ENTITIES)
	{
	    float32 rayIntersection = 0.0f;
	    glm::vec3 spherePos = glm::vec3(
		element.second.Position.x + element.second.Scale / 2,
		element.second.Position.y + element.second.Scale / 2,
		element.second.Position.z + element.second.Scale / 2);
	    
	    if (RaySphereIntersection(Camera->Position, rayWorld, spherePos, 0.5f, &rayIntersection))
	    {
	        globalContainerHoveredID = element.second.ID;
		break;
	    }
	    else
		globalContainerHoveredID = 0;
	}
	
	// I/O
	EngineUpdateTime(Engine->Time);
	InputStatePollEvents();
	if (InputState->Keyboard[GLFW_KEY_ESCAPE])
	    Engine->GlobalState = ENGINE_TERMINATE;
	if (InputState->Keyboard[GLFW_KEY_W])
	    CameraProcessKeyboard(Camera, FORWARD, Engine->Time->DeltaTime);
	if (InputState->Keyboard[GLFW_KEY_S])
	    CameraProcessKeyboard(Camera, BACKWARD, Engine->Time->DeltaTime);
	if (InputState->Keyboard[GLFW_KEY_A])
	    CameraProcessKeyboard(Camera, LEFT, Engine->Time->DeltaTime);
	if (InputState->Keyboard[GLFW_KEY_D])
	    CameraProcessKeyboard(Camera, RIGHT, Engine->Time->DeltaTime);	
	if (InputState->Keyboard[GLFW_KEY_SPACE])
	    CameraProcessKeyboard(Camera, UP, Engine->Time->DeltaTime);	
	if (InputState->Keyboard[GLFW_KEY_LEFT_CONTROL])
	    CameraProcessKeyboard(Camera, DOWN, Engine->Time->DeltaTime);	
	if (InputState->MouseLeftButton && !globalActiveWindow)
	{
	    InputStateUpdateMouseOffset(InputState);
	    CameraProcessMouseMovement(Camera,
				       InputState->MouseOffsetX,
				       InputState->MouseOffsetY);

	    if (globalContainerHoveredID != 0)
		globalContainerSelectedID = globalContainerHoveredID;
	    else
	        globalContainerSelectedID = 0;
	}

	// NOTE: START RENDERING ======================================
	RendererResetStats(Renderer);
        RendererStart(Renderer, CameraGetViewMatrix(Camera));

	// NOTE: CLASSIC RENDERING ======================================
	if (true)
	    RendererDrawDebugAxis(Renderer);	

	RendererStartNewBatchCube(Renderer);
	for (std::pair<uint32, entity_cube> element : GAME_TERRAIN_ENTITIES)
	{
	    if (globalTerrainHoveredID != 0 && globalTerrainHoveredID == element.second.ID)
	    {
		entity_cube hoveredSlot = EntityCubeConstruct(element.second.ID,
							      nullptr,
							      element.second.Position,
							      { 1.0f, 0.5f, 1.0f },
							      element.second.Scale,
							      { 0.7f, 0.7f, 0.7f, 1.0f },
							      element.second.State);

		PushEntityCubeToBuffer(Renderer, hoveredSlot, 1.0f);
	    } else
		PushEntityCubeToBuffer(Renderer, element.second, 1.0f);
	}
	//DrawSpheres();
	RendererCloseBatchCube(Renderer);
	RendererFlushBatchCube(Renderer);

	// NOTE: STENCIL RENDERING ======================================
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilMask(0xFF);

	RendererStartNewBatchCube(Renderer);
	for (std::pair<uint32, entity_cube> element : GAME_CONTAINER_ENTITIES)
	{
	    PushEntityCubeToBuffer(Renderer, element.second, 1.0f);
	}
	RendererCloseBatchCube(Renderer);
	RendererFlushBatchCube(Renderer);

	if (globalContainerHoveredID != 0 || globalContainerSelectedID != 0)
	{
	    RendererStartStencil(Renderer, CameraGetViewMatrix(Camera));

	    RendererStartNewBatchCube(Renderer);
	    if (globalContainerSelectedID != 0)
		PushEntityCubeToBuffer(Renderer, GAME_CONTAINER_ENTITIES[globalContainerSelectedID], 1.1f);
	    if (globalContainerHoveredID != 0)
		PushEntityCubeToBuffer(Renderer, GAME_CONTAINER_ENTITIES[globalContainerHoveredID], 1.1f);
	    RendererCloseBatchCube(Renderer);
	    RendererFlushBatchCube(Renderer);

	    RendererStopStencil();	    
	}
	
	// NOTE: UI RENDERING ======================================
        WrapImGuiNewFrame();	

        EngineShowOverlay(Engine);
        DrawSettingsPanel(Engine,
			  WIDTH, HEIGHT,
			  InputState,
			  Camera,
			  Renderer,
			  globalSliderMapSize,
			  GAME_CONTAINER_ENTITIES,
			  GAME_SLOT_RELATIONSHIPS,
			  globalActiveWindow);	

	WrapImGuiRender();
	
	// NOTE: SWAP BUFFER ======================================
	RendererSwapBufferAndFinish(Engine->Window);
    }

    CameraDelete(Camera);
    InputStateDelete(InputState);
    ShaderDeleteStorage();
    RendererDelete(Renderer);
    EngineDelete(Engine);
    return 0;
}

void CreateTestContainers()
{
    entity_cube containerOne = EntityCubeConstruct(1,
						   "Container1",
						   { 3.0f, 0.5f, -2.0f },
						   { 1.0f, 1.0f, 1.0f },
						   1.0f,
						   { 1.0f, 0.0f, 0.0f, 1.0f },
						   ENTITY_STATE_DYNAMIC);
    entity_cube containerTwo = EntityCubeConstruct(2,
						   "Container2",
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

void CreateTestTerrain(uint32 mapSize,
		       std::unordered_map<uint32, entity_cube> &terrain,
		       std::unordered_map<uint32, uint32> &slots)
{
    terrain.clear();
    //slots.clear(); // TODO: carefull overflow
    glm::vec4 color = { 0.2f, 0.2f, 0.2f, 1.0f };
    glm::vec4 colorH = { 0.25f, 0.25f, 0.25f, 1.0f };
    glm::vec3 size = { 1.0f, 0.5f, 1.0f };
    float32 scale = 1.0f;
    entity_state state = ENTITY_STATE_STATIC;
    float32 posX = 0.0f;
    bool slot = true;
    uint32 id = 1;
    int32 side = (int32)std::sqrt(mapSize);
    for (int32 i = 0; i < side; i++)
    {
	float32 posZ = -size.z;
	for (int32 y = 0; y < side; y++)
	{
	    glm::vec4 c = color;
	    entity_state s = state;
	    if (slot && i % 4 == 1)
	    {
		c = colorH;
		s = ENTITY_STATE_SLOT;
		if (slots[id] == 0)
		    slots[id] = 0;
	    }

	    entity_cube t = EntityCubeConstruct(id, nullptr, { posX, 0.0f, posZ }, size, scale, c, s);
	    terrain[t.ID] = t;
	    posZ -= size.z;
	    slot = !slot;
	    id++;
	}
	posX += size.x;
    }
}

void PushEntityCubeToBuffer(renderer *Renderer, entity_cube cube, float32 scale)
{
    RendererAddCubeToBuffer(Renderer,
		    cube.Position,
		    { cube.Size.x, cube.Size.y, cube.Size.z },
		    cube.Scale * scale,
		    cube.Color);
}

void DrawSettingsPanel(engine *Engine,
		       uint32 width, uint32 height,
		       input_state *InputState,
		       camera *Camera,
		       renderer *Renderer,
		       uint32 &mapSize,
		       std::unordered_map<uint32, entity_cube> &containers,
		       std::unordered_map<uint32, uint32> &slots,
		       bool &focus)
{
    ImGui::SetNextWindowPos(ImVec2(10, 10));
    ImGui::SetNextWindowSize(ImVec2(410, (float32)height - 20));
    ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_NoResize);

    EngineSettingsCollapseHeader(Engine, width, height);
    InputStateSettingsCollapseHeader(InputState);
    CameraSettingsCollapseHeader(Camera);
    RendererSettingsCollapseHeader(Renderer);

    // World
    if (ImGui::CollapsingHeader("World settings", ImGuiTreeNodeFlags_DefaultOpen))
    {
	ImGui::Text("slots: %03d/%03d", 0, slots.size());
	if (ImGui::SliderInt("floor", &(int)mapSize, 0, 1000))
	    slots.clear();
	ImGui::Separator();
    }

    // Containers
    if (ImGui::CollapsingHeader("Object settings", ImGuiTreeNodeFlags_DefaultOpen))
    {
	static uint32 selected = 0;
	ImGui::BeginChild("left pane", ImVec2(120, 150));
	for (std::pair<uint32, entity_cube> element : containers)	 
	{
	    char label[128];
	    sprintf_s(label, "obj: <%s>", element.second.Name);
	    if (ImGui::Selectable(label, (uint32)globalContainerSelectedID == element.first))
		globalContainerSelectedID = element.first;
	}

	ImGui::EndChild();
	ImGui::SameLine();

	ImGui::BeginChild("right pane", ImVec2(0, 150));
	if (globalContainerSelectedID != 0)
	{
	    ImGui::Text("mem: %p", &containers[globalContainerSelectedID]);
	    ImGui::Text("ID: %03d", containers[globalContainerSelectedID].ID);
	    ImGui::Text("Name: %s", containers[globalContainerSelectedID].Name);
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
    static uint32 selectedSlot = 0;
    if (ImGui::CollapsingHeader("Slots settings", ImGuiTreeNodeFlags_DefaultOpen))
    {
	ImGui::Columns(2);
	for (std::pair<uint32, uint32> sl : slots)
	{
	    char buf1[32];
	    sprintf_s(buf1, "%03d", sl.first);
	    ImGui::Button(buf1, ImVec2(-FLT_MIN, 0.0f));
	    ImGui::NextColumn();

	    char buf2[32];
	    if (sl.second == 0)
		sprintf_s(buf2, "%03d<empty>", sl.first);
	    else
		sprintf_s(buf2, "%03d", sl.second);

	    if (ImGui::Button(buf2, ImVec2(-FLT_MIN, 0.0f)))
	    {
		selectedSlot = sl.first;
		ImGui::OpenPopup("objects_popup");
	    }

	    ImGui::NextColumn();
	}

	ImGui::Columns(1);
	ImGui::Separator();
    }

    if (ImGui::BeginPopup("objects_popup"))
    {
	ImGui::Text("slot-%03d", selectedSlot);
	ImGui::Separator();
	for (std::pair<uint32, entity_cube> ct : containers)	 
	{
	    if (ImGui::Selectable(containers[ct.first].Name))
		AttribContainerToSlot(slots, containers, selectedSlot, ct.first);
	}
	ImGui::EndPopup();
    }
    
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow))
	focus = true;
    else
	focus = false;
    ImGui::End();
}

void AttribContainerToSlot(std::unordered_map<uint32, uint32> &slots,
			   std::unordered_map<uint32, entity_cube> &containers,
			   uint32 selectedSlot,
			   uint32 selectedItem)
{
    for (std::pair<uint32, uint32> sl : slots)
    {
	if (sl.second == selectedItem)
	{
	    slots[sl.first] = 0;
	    break;
	}
    }

    if (slots.find(selectedSlot) != slots.end())
    {
	if (containers.find(selectedItem) != containers.end())
	{
	    slots[selectedSlot] = selectedItem;
	}
    }
    
}

// TODO: move to entity (entity_sphere...)
unsigned int SphereVAO, SphereVBO, SphereIBO;
std::vector<int> sphereIndices;
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
