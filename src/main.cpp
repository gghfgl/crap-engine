#include <iostream>

#include "plateform.h"

/* TODO:
   - load assets (models, sound, skybox, etc ...)
   - create and save level from editor
   - move from editing to running scene on the fly
   - scene = access engine API for creating game design
   - beautyfull light PBR effects
   - custom GUI for game UI

   - learn more compiler stuff
   - learn how properly debug with MSVC
*/

bool g_TestActive = true;
const uint32 g_Width            = 1280;
const uint32 g_Height           = 960;
static bool g_ActiveWindow      = false;
static uint32 g_SliderMapSize   = 130;
static uint32 g_CacheSliderMapSize   = 130;
uint32 g_ContainerSelectedID    = 0;
uint32 g_ContainerHoveredID     = 0;
uint32 g_TerrainHoveredID       = 0;

glm::vec3 g_CameraStartPosition = glm::vec3(0.0f, 5.0f, 10.0f);

std::unordered_map<uint32, entity_cube> GAME_TERRAIN_ENTITIES;
std::unordered_map<uint32, entity_cube> GAME_CONTAINER_ENTITIES;
std::unordered_map<uint32, uint32> GAME_SLOTS_RELATIONSHIP;

//void DrawSettingsPanel(engine *Engine, uint32 width, uint32 height, input_state *InputState, camera *Camera, renderer *Renderr, uint32 &mapSize, std::unordered_map<uint32, entity_cube> &objects, std::unordered_map<uint32, uint32> &slots, bool &focus);
void AttribContainerToSlot(std::unordered_map<uint32, uint32> &slots, std::unordered_map<uint32, entity_cube> &containers, uint32 selectedSlot, uint32 selectedItem);

void CreateTestTerrain(uint32 mapSize, std::unordered_map<uint32, entity_cube> &terrain, std::unordered_map<uint32, uint32> &slots);
void CreateTestContainers(std::unordered_map<uint32, entity_cube> &containers);
void CreateTestSpheres(float radius, int slacks, int slices); // TODO: entity_sphere

void PushEntityCubeToBuffer(renderer *Renderer, entity_cube cube, float32 scale);
void DrawSpheres();

// NOTE: ENGINE LEVEL EDITOR
int main(int argc, char *argv[])
{
    // Init all systems
    window_t *Window = window::Construct(g_Width, g_Height, "crapEngine");
    input_t *InputState = input::Construct(Window->PlatformWindow);
    camera_t *Camera = camera::Construct((float32)g_Width, (float32)g_Height, g_CameraStartPosition);

    // TODO: draw gride from shader?
    shader::CompileAndCache("../shaders/default.vs", "../shaders/default.fs", nullptr,
			  "default", Camera->ProjectionMatrix);
    shader::CompileAndCache("../shaders/default.vs", "../shaders/stencil.fs", nullptr,
			  "stencil", Camera->ProjectionMatrix);

    // TODO: renderer::CreateAndInit()
    renderer *Renderer = RendererConstruct();

    // TODO: use engine state to fill debug VAO / VBO and draw
    RendererPrepareDebugAxis(Renderer);

    // TODO: move batch rendering away atm
    RendererPrepareCubeBatching(Renderer);

    // TODO: load model and texture now. Fuck primitiv atm!
    CreateTestContainers(GAME_CONTAINER_ENTITIES);
    //CreateTestSpheres(0.5f, 20, 20); // dirty way to create spheres
    CreateTestTerrain(g_SliderMapSize, GAME_TERRAIN_ENTITIES, GAME_SLOTS_RELATIONSHIP);

    while (g_TestActive)
    {
	window::UpdateTime(Window->Time);
	// window::UpdateSize(Window); // TODO: useless?

	if (g_SliderMapSize != g_CacheSliderMapSize)
	{
	    CreateTestTerrain(g_SliderMapSize, GAME_TERRAIN_ENTITIES, GAME_SLOTS_RELATIONSHIP);
	    g_CacheSliderMapSize = g_SliderMapSize;
	}

	// NOTE: READ INPUTS ======================================
	input::PollEvents();
	if (InputState->KeyboardEvent->IsPressed[CRAP_KEY_ESCAPE])
	    g_TestActive = false;
	if (InputState->KeyboardEvent->IsPressed[CRAP_KEY_W])
	    camera::ProcessMovementDirectional(Camera, FORWARD, Window->Time->DeltaTime);
	if (InputState->KeyboardEvent->IsPressed[CRAP_KEY_S])
	    camera::ProcessMovementDirectional(Camera, BACKWARD, Window->Time->DeltaTime);
	if (InputState->KeyboardEvent->IsPressed[CRAP_KEY_A])
	    camera::ProcessMovementDirectional(Camera, LEFT, Window->Time->DeltaTime);
	if (InputState->KeyboardEvent->IsPressed[CRAP_KEY_D])
	    camera::ProcessMovementDirectional(Camera, RIGHT, Window->Time->DeltaTime);
	if (InputState->KeyboardEvent->IsPressed[CRAP_KEY_SPACE])
	    camera::ProcessMovementDirectional(Camera, UP, Window->Time->DeltaTime);
	if (InputState->KeyboardEvent->IsPressed[CRAP_KEY_LEFT_CONTROL])
	    camera::ProcessMovementDirectional(Camera, DOWN, Window->Time->DeltaTime);

	if (InputState->MouseEvent->LeftButton && !g_ActiveWindow)
	{
	    input::UpdateMouseOffsets(InputState->MouseEvent);
	    camera::ProcessMovementAngles(Camera,
				       InputState->MouseEvent->OffsetX,
				       InputState->MouseEvent->OffsetY);
	}

	// NOTE: SIMULATE WORLD ======================================
	// Ray casting test
	// glm::vec3 rayWorld = MouseRayDirectionWorld((float32)InputState->MousePosX,
	// 					    (float32)InputState->MousePosY,
	// 					    WIDTH,
	// 					    HEIGHT,
	// 					    Camera->ProjectionMatrix,
	// 					    CameraGetViewMatrix(Camera));

	// for (std::pair<uint32, entity_cube> element : GAME_TERRAIN_ENTITIES)
	// {
	//     float32 rayIntersection = 0.0f;
	//     glm::vec3 spherePos = glm::vec3(
	// 	element.second.Position.x + element.second.Scale / 2,
	// 	element.second.Position.y + element.second.Scale / 2,
	// 	element.second.Position.z + element.second.Scale / 2);
	    
	//     if (RaySphereIntersection(Camera->Position, rayWorld, spherePos, 0.5f, &rayIntersection))
	//     {
	// 	if (element.second.State == ENTITY_STATE_SLOT)
	// 	{
	// 	    g_TerrainHoveredID = element.second.ID;
	// 	    break;
	// 	}
	//     }
	//     else
	// 	g_TerrainHoveredID = 0;
	// }

	// for (std::pair<uint32, entity_cube> element : GAME_CONTAINER_ENTITIES)
	// {
	//     float32 rayIntersection = 0.0f;
	//     glm::vec3 spherePos = glm::vec3(
	// 	element.second.Position.x + element.second.Scale / 2,
	// 	element.second.Position.y + element.second.Scale / 2,
	// 	element.second.Position.z + element.second.Scale / 2);
	    
	//     if (RaySphereIntersection(Camera->Position, rayWorld, spherePos, 0.5f, &rayIntersection))
	//     {
	//         g_ContainerHoveredID = element.second.ID;
	// 	break;
	//     }
	//     else
	// 	g_ContainerHoveredID = 0;
	// }

	
	// NOTE: START RENDERING ======================================
	RendererResetStats(Renderer);
        RendererStart(Renderer, shader::GetFromCache("default"), camera::GetViewMatrix(Camera));

	// ** classic
	// TODO if (Engine->DebugMode) ?
	RendererDrawDebugAxis(Renderer);	

	RendererStartNewBatchCube(Renderer);
	for (std::pair<uint32, entity_cube> element : GAME_TERRAIN_ENTITIES)
	{
	    if (g_TerrainHoveredID != 0 && g_TerrainHoveredID == element.second.ID)
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
	
	// ** stencil
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilMask(0xFF); //enable writing stencil buffer

	RendererStartNewBatchCube(Renderer);
	for (std::pair<uint32, entity_cube> element : GAME_CONTAINER_ENTITIES)
	{
	    glClear(GL_STENCIL_BUFFER_BIT);
	    PushEntityCubeToBuffer(Renderer, element.second, 1.0f);
	}

	RendererCloseBatchCube(Renderer);
	RendererFlushBatchCube(Renderer);

	if (g_ContainerHoveredID != 0 || g_ContainerSelectedID != 0)
	{
	    RendererStartStencil(Renderer, shader::GetFromCache("stencil"), camera::GetViewMatrix(Camera));

	    RendererStartNewBatchCube(Renderer);
	    if (g_ContainerSelectedID != 0)
	    {
		//RendererStartNewBatchCube(Renderer);

		//glClear(GL_STENCIL_BUFFER_BIT);
		//RendererStartStencil(Renderer, ShaderGetFromStorage("stencil"), CameraGetViewMatrix(Camera));
		PushEntityCubeToBuffer(Renderer, GAME_CONTAINER_ENTITIES[g_ContainerSelectedID], 1.1f);
		//RendererStopStencil();	    

		//RendererCloseBatchCube(Renderer);
		//RendererFlushBatchCube(Renderer);
	    }
	    if (g_ContainerHoveredID != 0)
	    {
		//RendererStartNewBatchCube(Renderer);

		//glClear(GL_STENCIL_BUFFER_BIT);
		//RendererStartStencil(Renderer, ShaderGetFromStorage("stencil"), CameraGetViewMatrix(Camera));
		PushEntityCubeToBuffer(Renderer, GAME_CONTAINER_ENTITIES[g_ContainerHoveredID], 1.1f);
		//RendererStopStencil();	    

		//RendererCloseBatchCube(Renderer);
		//RendererFlushBatchCube(Renderer);
	    }

	    RendererCloseBatchCube(Renderer);
	    RendererFlushBatchCube(Renderer);

	    RendererStopStencil();	    
	}
	
	// ** UI
        // WrapImGuiNewFrame();	

        // EngineShowOverlay(Engine);
        // DrawSettingsPanel(Engine,
	// 		  WIDTH, HEIGHT,
	// 		  InputState,
	// 		  Camera,
	// 		  Renderer,
	// 		  g_SliderMapSize,
	// 		  GAME_CONTAINER_ENTITIES,
	// 		  GAME_SLOTS_RELATIONSHIP,
	// 		  g_ActiveWindow);

	//WrapImGuiRender();
	
	// ** Swap buffer
	// TODO:  update memory pool
	Renderer->MemoryArena->MaxUsed = 0;
	window::SwapBuffer(Window);
    }

    RendererDelete(Renderer);

    shader::ClearCache();
    camera::Delete(Camera);
    input::Delete(InputState);
    window::Delete(Window);
    return 0;
}

void CreateTestContainers(std::unordered_map<uint32, entity_cube> &containers)
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
    
    if (containers.find(containerOne.ID) == containers.end())
	containers[containerOne.ID] = containerOne;
    if (containers.find(containerTwo.ID) == containers.end())
	containers[containerTwo.ID] = containerTwo;
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
    RendererAddCubeToBuffer(Renderer, cube.Position, { cube.Size.x, cube.Size.y, cube.Size.z },
			    cube.Scale * scale, cube.Color);
}

// TODO: move to GUI
// void DrawSettingsPanel(engine *Engine,
// 		       uint32 width, uint32 height,
// 		       input_state *InputState,
// 		       camera *Camera,
// 		       renderer *Renderer,
// 		       uint32 &mapSize,
// 		       std::unordered_map<uint32, entity_cube> &containers,
// 		       std::unordered_map<uint32, uint32> &slots,
// 		       bool &focus)
// {
//     ImGui::SetNextWindowPos(ImVec2(10, 10));
//     ImGui::SetNextWindowSize(ImVec2(410, (float32)height - 20));
//     ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_NoResize);

//     EngineSettingsCollapseHeader(Engine, width, height);
//     input::InputStateSettingsCollapseHeader(InputState);
//     CameraSettingsCollapseHeader(Camera);
//     RendererSettingsCollapseHeader(Renderer);

//     // World
//     if (ImGui::CollapsingHeader("World settings", ImGuiTreeNodeFlags_DefaultOpen))
//     {
// 	ImGui::Text("slots: %03d/%03d", 0, slots.size());
// 	if (ImGui::SliderInt("floor", &(int)mapSize, 0, 1000))
// 	    slots.clear();
// 	ImGui::Separator();
//     }

//     // Containers
//     if (ImGui::CollapsingHeader("Object settings", ImGuiTreeNodeFlags_DefaultOpen))
//     {
// 	static uint32 selected = 0;
// 	ImGui::BeginChild("left pane", ImVec2(120, 150));
// 	for (std::pair<uint32, entity_cube> element : containers)	 
// 	{
// 	    char label[128];
// 	    sprintf_s(label, "obj: <%s>", element.second.Name);
// 	    if (ImGui::Selectable(label, (uint32)g_ContainerSelectedID == element.first))
// 		g_ContainerSelectedID = element.first;
// 	}

// 	ImGui::EndChild();
// 	ImGui::SameLine();

// 	ImGui::BeginChild("right pane", ImVec2(0, 150));
// 	if (g_ContainerSelectedID != 0)
// 	{
// 	    ImGui::Text("mem: %p", &containers[g_ContainerSelectedID]);
// 	    ImGui::Text("ID: %03d", containers[g_ContainerSelectedID].ID);
// 	    ImGui::Text("Name: %s", containers[g_ContainerSelectedID].Name);
// 	    ImGui::Text("State: %s",
// 			(containers[g_ContainerSelectedID].State == ENTITY_STATE_STATIC ? "STATIC" : "DYNAMIC"));
// 	    ImGui::Text("Pos x=%.2f y=%.2f z=%.2f",
// 			containers[g_ContainerSelectedID].Position.x,
// 			containers[g_ContainerSelectedID].Position.y,
// 			containers[g_ContainerSelectedID].Position.z);

// 	    ImGui::Text("Size x=%.2f y=%.2f z=%.2f w=%.2f",
// 			containers[g_ContainerSelectedID].Size.x,
// 			containers[g_ContainerSelectedID].Size.y,
// 			containers[g_ContainerSelectedID].Size.z,
// 			containers[g_ContainerSelectedID].Color.w);

// 	    ImGui::Text("Color r=%.2f g=%.2f b=%.2f a=%.2f",
// 			containers[g_ContainerSelectedID].Color.r,
// 			containers[g_ContainerSelectedID].Color.g,
// 			containers[g_ContainerSelectedID].Color.b,
// 			containers[g_ContainerSelectedID].Color.a);
// 	}

// 	ImGui::EndChild();
// 	ImGui::Separator();
//     }

//     // Slots
//     static uint32 selectedSlot = 0;
//     if (ImGui::CollapsingHeader("Slots settings", ImGuiTreeNodeFlags_DefaultOpen))
//     {
// 	ImGui::Columns(2);
// 	for (std::pair<uint32, uint32> sl : slots)
// 	{
// 	    char buf1[32];
// 	    sprintf_s(buf1, "%03d", sl.first);
// 	    ImGui::Button(buf1, ImVec2(-FLT_MIN, 0.0f));
// 	    ImGui::NextColumn();

// 	    char buf2[32];
// 	    if (sl.second == 0)
// 		sprintf_s(buf2, "%03d<empty>", sl.first);
// 	    else
// 		sprintf_s(buf2, "%03d", sl.second);

// 	    if (ImGui::Button(buf2, ImVec2(-FLT_MIN, 0.0f)))
// 	    {
// 		selectedSlot = sl.first;
// 		ImGui::OpenPopup("objects_popup");
// 	    }

// 	    ImGui::NextColumn();
// 	}

// 	ImGui::Columns(1);
// 	ImGui::Separator();
//     }

//     if (ImGui::BeginPopup("objects_popup"))
//     {
// 	ImGui::Text("slot-%03d", selectedSlot);
// 	ImGui::Separator();
// 	for (std::pair<uint32, entity_cube> ct : containers)	 
// 	{
// 	    if (ImGui::Selectable(containers[ct.first].Name))
// 		AttribContainerToSlot(slots, containers, selectedSlot, ct.first);
// 	}
// 	ImGui::EndPopup();
//     }
    
//     if (ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow))
// 	focus = true;
//     else
// 	focus = false;
//     ImGui::End();
// }

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
