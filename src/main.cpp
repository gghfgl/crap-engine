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
	
static bool activeWindow = false;
static int MAPSIZE = 10;
static int sliderMapSize = 10;
unsigned int selectedID = 0;
unsigned int hoveredID = 0;
unsigned int terrainHoveredID = 0;
static int SLOTS_COUNT = 0;
std::unordered_map<int, entity_cube> GAME_TERRAIN_ENTITIES;
std::unordered_map<int, entity_cube> GAME_CONTAINER_ENTITIES;

void DrawSettingsPanel(engine *Engine, int &mapSize, int mPosX, int mPosY, std::unordered_map<int, entity_cube> &objects, bool &focus);
glm::vec3 MouseRayDirectionWorld(float mouseX, float mouseY, int width, int height, glm::mat4 projectionMatrix, glm::mat4 viewMatrix);
bool RaySphereIntersection(glm::vec3 rayOriginWorld, glm::vec3 rayDirectionWorld, glm::vec3 sphereCenterWorld, float sphereRadius, float* intersectionDistance);

void CreateTestTerrain(int mapSize, int &slotsCount); // NOTE: entity_cube
void CreateTestContainers(); // NOTE: entity_cube
void CreateTestSpheres(float radius, int slacks, int slices); // NOTE: sphere

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
	if (sliderMapSize % 2 == 0)
	    MAPSIZE = sliderMapSize;
	CreateTestTerrain(MAPSIZE, SLOTS_COUNT);

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
		    terrainHoveredID = element.second.ID;
		    break;
		}
	    }
	    else
		terrainHoveredID = 0;
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
	        hoveredID = element.second.ID;
		break;
	    }
	    else
		hoveredID = 0;
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
	if (Engine->InputState->MouseLeftButton && !activeWindow)
	{
	    UpdateMouseOffset(Engine->InputState);
	    ProcessCameraMouseMovement(Engine->Camera,
				       Engine->InputState->MouseOffsetX,
				       Engine->InputState->MouseOffsetY);

	    if (hoveredID != 0)
		selectedID = hoveredID;
	    else
	        selectedID = 0;
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
	    if (terrainHoveredID != 0 && terrainHoveredID == element.second.ID)
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

	if (hoveredID != 0 || selectedID != 0)
	{
	    StartStencilRendering(Engine->Renderer, Engine->Camera);

	    StartNewBatchCube(Engine->Renderer);
	    if (selectedID != 0)
		PushEntityCubeToBuffer(Engine->Renderer, GAME_CONTAINER_ENTITIES[selectedID], 1.1f);
	    if (hoveredID != 0)
		PushEntityCubeToBuffer(Engine->Renderer, GAME_CONTAINER_ENTITIES[hoveredID], 1.1f);
	    CloseBatchCube(Engine->Renderer);
	    FlushBatchCube(Engine->Renderer);

	    StopRenderingStencil();	    
	}
	
	// NOTE: UI RENDERING ======================================
	StartImGuiRendering();	

	DrawDebugOverlay(Engine);
        DrawSettingsPanel(Engine,
			  sliderMapSize,
			  (int)Engine->InputState->MousePosX,
			  (int)Engine->InputState->MousePosY,
			  GAME_CONTAINER_ENTITIES,
			  activeWindow);	

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
						   { 3.0f, 0.5f, 0.0f },
						   { 1.0f, 1.0f, 1.0f },
						   1.0f,
						   { 1.0f, 0.0f, 0.0f, 1.0f },
						   ENTITY_STATE_DYNAMIC);
    entity_cube containerTwo = EntityCubeConstruct(2,
						   { -3.0f, 0.5f, 0.0f },
						   { 1.0f, 1.0f, 1.0f },
						   1.0f,
						   { 0.0f, 0.0f, 1.0f, 1.0f },
						   ENTITY_STATE_DYNAMIC);
    
    if (GAME_CONTAINER_ENTITIES.find(containerOne.ID) == GAME_CONTAINER_ENTITIES.end())
	GAME_CONTAINER_ENTITIES[containerOne.ID] = containerOne;
    if (GAME_CONTAINER_ENTITIES.find(containerTwo.ID) == GAME_CONTAINER_ENTITIES.end())
	GAME_CONTAINER_ENTITIES[containerTwo.ID] = containerTwo;
}

void CreateTestTerrain(int mapSize, int &slotsCount)
{
    slotsCount = 0;
    GAME_TERRAIN_ENTITIES.clear();
    glm::vec4 color = { 0.2f, 0.2f, 0.2f, 1.0f };
    glm::vec4 colorH = { 0.25f, 0.25f, 0.25f, 1.0f };
    glm::vec3 size = { 1.0f, 0.5f, 1.0f };
    float scale = 1.0f;
    entity_state state = ENTITY_STATE_STATIC;
    float posX = 0.0f;
    bool slot = true;
    unsigned int id = 1;
    for (int i = 0; i < mapSize / 2; i++)
    {
	float posZ = -size.z;
	for (int y = 0; y < mapSize / 2; y++)
	{
	    glm::vec4 c = color;
	    entity_state s = state;
	    if (slot)
	    {
		c = colorH;
		s = ENTITY_STATE_SLOT;
		slotsCount++;
	    }

	    entity_cube t = EntityCubeConstruct(id, { posX, 0.0f, posZ }, size, scale, c, s);
	    GAME_TERRAIN_ENTITIES[t.ID] = t;
	    posZ -= size.z;
	    slot = !slot;
	    id++;
	}
	posX += size.x;
    }

    posX = -1.0f;
    for (int i = 0; i < mapSize / 2; i++)
    {
	float posZ = -size.z;
	for (int y = 0; y < mapSize / 2; y++)
	{
	    glm::vec4 c = color;
	    entity_state s = state;
	    if (slot)
	    {
		c = colorH;
		s = ENTITY_STATE_SLOT;
		slotsCount++;
	    }

	    entity_cube t = EntityCubeConstruct(id, { posX, 0.0f, posZ }, size, scale, c, s);
	    GAME_TERRAIN_ENTITIES[t.ID] = t;
	    posZ -= size.z;
	    slot = !slot;
	    id++;
	}
	posX -= size.x;
    }

    slot = false;
    posX = 0.0f;
    for (int i = 0; i < mapSize / 2; i++)
    {
	float posZ = 0.0f;
	for (int y = 0; y < mapSize / 2; y++)
	{
	    glm::vec4 c = color;
	    entity_state s = state;
	    if (slot)
	    {
		c = colorH;
		s = ENTITY_STATE_SLOT;
		slotsCount++;
	    }

	    entity_cube t = EntityCubeConstruct(id, { posX, 0.0f, posZ }, size, scale, c, s);
	    GAME_TERRAIN_ENTITIES[t.ID] = t;
	    posZ += size.z;
	    slot = !slot;
	    id++;
	}
	posX += size.x;
    }

    posX = -1.0f;
    for (int i = 0; i < mapSize / 2; i++)
    {
	float posZ = 0.0f;
	for (int y = 0; y < mapSize / 2; y++)
	{
	    glm::vec4 c = color;
	    entity_state s = state;
	    if (slot)
	    {
		c = colorH;
		s = ENTITY_STATE_SLOT;
		slotsCount++;
	    }

	    entity_cube t = EntityCubeConstruct(id, { posX, 0.0f, posZ }, size, scale, c, s);
	    GAME_TERRAIN_ENTITIES[t.ID] = t;
	    posZ += size.z;
	    slot = !slot;
	    id++;
	}
	posX -= size.x;
    }
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
		       int mPosX, int mPosY,
		       std::unordered_map<int, entity_cube> &objects,
		       bool &focus)
{
    ImGui::SetNextWindowPos(ImVec2(10, 10));
    ImGui::SetNextWindowSize(ImVec2(410, 700));
    ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_NoResize);

    if (ImGui::CollapsingHeader("Engine settings", ImGuiTreeNodeFlags_DefaultOpen))
    {
	ImGui::Text("screen: %d x %d", Engine->Width, Engine->Height);
	ImGui::Text("mouseX: %d / mouseY: %d", mPosX, mPosY);
    }

    if (ImGui::CollapsingHeader("Render settings", ImGuiTreeNodeFlags_DefaultOpen))
    {
	ImGui::Text("maxCube/draw: %d", globalMaxCubeCount); // TODO
	ImGui::Text("cubes: %d", Engine->Renderer->Stats.CubeCount);
	ImGui::Text("draws: %d", Engine->Renderer->Stats.DrawCount);
    }

    if (ImGui::CollapsingHeader("Camera settings", ImGuiTreeNodeFlags_DefaultOpen))
    {
	ImGui::Text("yaw: %.2f", globalYawSetting); // TODO
	ImGui::Text("pitch: %.2f", globalPitchSetting); // TODO
	ImGui::Text("speed: %.2f", globalSpeedSetting); // TODO
	ImGui::Text("sensitivity: %.2f", globalSensitivitySetting); // TODO
	ImGui::Text("fov: %.2f", globalFovSetting); // TODO
    }

    if (ImGui::CollapsingHeader("World settings", ImGuiTreeNodeFlags_DefaultOpen))
    {
	ImGui::Text("slots: %d/%d", 0, SLOTS_COUNT);
	ImGui::SliderInt("floor", &mapSize, 0, 100);
	ImGui::Separator();
    }

    if (ImGui::CollapsingHeader("Object settings", ImGuiTreeNodeFlags_DefaultOpen))
    {
    // left
    static int selected = 0;
    ImGui::BeginChild("left pane", ImVec2(150, 0), true);

    for (std::pair<int, entity_cube> element : objects)	 
    {
    	char label[128];
    	sprintf_s(label, "obj %d", element.first);
    	if (ImGui::Selectable(label, (int)selectedID == element.first))
    	    selectedID = element.first;
    }

    ImGui::EndChild();
    ImGui::SameLine();

    // right
    ImGui::BeginGroup();
    //ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing())); // Leave room for 1 line below us
    ImGui::BeginChild("item view");
    ImGui::Text("obj: %d", selectedID);
    ImGui::Separator();
    if (selectedID != 0)
    {
    	ImGui::Text("ID: %d", objects[selectedID].ID);
    	ImGui::Text("mem: %p", &objects[selectedID]);
    	ImGui::Text("State: %s",
    		    (objects[selectedID].State == ENTITY_STATE_STATIC ? "STATIC" : "DYNAMIC"));
    	ImGui::Text("Pos x=%.2f y=%.2f z=%.2f",
    		    objects[selectedID].Position.x,
    		    objects[selectedID].Position.y,
    		    objects[selectedID].Position.z);

    	ImGui::Text("Size x=%.2f y=%.2f z=%.2f w=%.2f",
    		    objects[selectedID].Size.x,
    		    objects[selectedID].Size.y,
    		    objects[selectedID].Size.z,
    		    objects[selectedID].Color.w);

    	ImGui::Text("Color r=%.2f g=%.2f b=%.2f a=%.2f",
    		    objects[selectedID].Color.r,
    		    objects[selectedID].Color.g,
    		    objects[selectedID].Color.b,
    		    objects[selectedID].Color.a);
    }
    ImGui::EndChild();
    ImGui::EndGroup();
    }
    if (ImGui::IsWindowFocused())
	focus = true;
    else
	focus = false;
    ImGui::End();
}

glm::vec3 MouseRayDirectionWorld(float mouseX,float mouseY,
				 int width, int height,
				 glm::mat4 projectionMatrix,
				 glm::mat4 viewMatrix)
{
    // transform to NDC
    float mx = (2.0f * mouseX) / width - 1.0f;
    float my = 1.0f - (2.0f * mouseY) / height;
    float mz = 1.0f;
    glm::vec3 rayNDC = glm::vec3(mx, my, mz);

    glm::vec4 rayClip = glm::vec4(rayNDC.x, rayNDC.y, -1.0, 1.0);
    glm::vec4 rayEye = inverse(projectionMatrix) * rayClip;
    rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0, 0.0);

    glm::vec4 stepWorld = inverse(viewMatrix) * rayEye;
    glm::vec3 rayWorld = glm::vec3(stepWorld.x, stepWorld.y, stepWorld.z);

    return normalize(rayWorld);
}

bool RaySphereIntersection(glm::vec3 rayOriginWorld,
			       glm::vec3 rayDirectionWorld,
			       glm::vec3 sphereCenterWorld,
			       float sphereRadius,
			       float* intersectionDistance)
{
    // work out components of quadratic
    glm::vec3 distToSphere = rayOriginWorld - sphereCenterWorld;
    float b = dot( rayDirectionWorld, distToSphere );
    float c = dot( distToSphere, distToSphere ) - sphereRadius * sphereRadius;
    float b_squared_minus_c = b * b - c;

    // check for "imaginary" answer. == ray completely misses sphere
    if ( b_squared_minus_c < 0.0f ) { return false; }

    // check for ray hitting twice (in and out of the sphere)
    if ( b_squared_minus_c > 0.0f ) {
	// get the 2 intersection distances along ray
	float t_a              = -b + sqrt( b_squared_minus_c );
	float t_b              = -b - sqrt( b_squared_minus_c );
	*intersectionDistance = t_b;

	// if behind viewer, throw one or both away
	if ( t_a < 0.0 ) {
	    if ( t_b < 0.0 ) { return false; }
	} else if ( t_b < 0.0 ) {
	    *intersectionDistance = t_a;
	}

	return true;
    }

    // check for ray hitting once (skimming the surface)
    if ( 0.0f == b_squared_minus_c ) {
	// if behind viewer, throw away
	float t = -b + sqrt( b_squared_minus_c );
	if ( t < 0.0f ) { return false; }
	*intersectionDistance = t;
	return true;
    }

    // note: could also check if ray origin is inside sphere radius
    return false;
}

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
