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
	
#define RGB_WHITE (0xFF | (0xFF<<8) | (0xFF<<16))

static bool activeWindow = false;
static int MAPSIZE = 10;
int selectedID = 0;
int hoveredID = 0;

std::unordered_map<int, entity_cube> CONTAINER_ENTITIES;

glm::vec4 GetColorByIndex(int index);
void CrapColors(float *r, float *g, float *b);
void DrawTerrain(renderer *Renderer, int mapSize);
void DrawCubeContainer(renderer *Renderer, entity_cube container, float scale);
void DrawTerrainTool(engine *Engine, int &mapSize, int mPosX, int mPosY, bool &focus);
void DrawObjectPanel(std::unordered_map<int, entity_cube> &objects, bool &focus);
glm::vec3 MouseRayDirectionWorld(float mouseX, float mouseY, int width, int height, glm::mat4 projectionMatrix, glm::mat4 viewMatrix);
bool TestRaySphereIntersection(glm::vec3 rayOriginWorld, glm::vec3 rayDirectionWorld, glm::vec3 sphereCenterWorld, float sphereRadius, float* intersectionDistance);

void CreateTestContainers();
void CreateTestSpheres(float radius, int slacks, int slices);
void DrawSpheres();

int main(int argc, char *argv[])
{
    engine *Engine = EngineConstructAndInit(1280, 960, DEBUG_MODE | VSYNC);
    PrepareEmbededAxisDebugRendering(Engine->Renderer);
    PrepareCubeBatchRendering(Engine->Renderer);
    
    // TODO: CreateTerrain(MAPSIZE) with slot parameter (default settings should come from file)
    CreateTestContainers(); // quick way to create test cube (should come from file)
    //CreateTestSpheres(0.5f, 20, 20); // dirty way to create spheres

    while (Engine->GlobalState == ENGINE_ACTIVE)
    {
	// Ray casting test
	glm::vec3 rayWorld = MouseRayDirectionWorld((float)Engine->InputState->MousePosX,
						    (float)Engine->InputState->MousePosY,
						    Engine->Width,
						    Engine->Height,
						    Engine->ProjMatrix,
						    GetCameraViewMatrix(Engine->Camera));

	for (std::pair<int, entity_cube> element : CONTAINER_ENTITIES)
	{
	    float rayIntersection = 0.0f;
	    glm::vec3 spherePos = glm::vec3(
		element.second.Position.x + element.second.Size.w / 2,
		element.second.Position.y + element.second.Size.w / 2,
		element.second.Position.z + element.second.Size.w / 2);
	    
	    if (TestRaySphereIntersection(Engine->Camera->Position, rayWorld, spherePos, 0.5f, &rayIntersection))
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

	// TODO: loop through TERRAIN_ENTITIES and draw (add to buffer)
	// terrain cubes. {inside batch fuctions}
	
	DrawTerrain(Engine->Renderer, MAPSIZE);
	// DrawSpheres();

	// NOTE: STENCIL RENDERING ======================================
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilMask(0xFF);

	StartNewBatchCube(Engine->Renderer);
	for (std::pair<int, entity_cube> element : CONTAINER_ENTITIES)
	{
	    DrawCubeContainer(Engine->Renderer, element.second, 1.0f);
	}
	CloseBatchCube(Engine->Renderer);
	FlushBatchCube(Engine->Renderer);

	// TODO: add terrain slot hovered stencil draw
	if (hoveredID != 0 || selectedID != 0)
	{
	    StartStencilRendering(Engine->Renderer, Engine->Camera);

	    StartNewBatchCube(Engine->Renderer);
	    if (selectedID != 0)
		DrawCubeContainer(Engine->Renderer, CONTAINER_ENTITIES[selectedID], 1.1f);
	    if (hoveredID != 0)
		DrawCubeContainer(Engine->Renderer, CONTAINER_ENTITIES[hoveredID], 1.1f);
	    CloseBatchCube(Engine->Renderer);
	    FlushBatchCube(Engine->Renderer);

	    StopRenderingStencil();	    
	}

	// NOTE: UI RENDERING ======================================
	StartImGuiRendering();	

	DrawDebugOverlay(Engine);
        DrawTerrainTool(Engine,
			MAPSIZE,
			(int)Engine->InputState->MousePosX,
			(int)Engine->InputState->MousePosX,
        		activeWindow);
        DrawObjectPanel(CONTAINER_ENTITIES, activeWindow);	

	RenderImGui();
	
	// NOTE: SWAP BUFFER ======================================
	SwapBufferAndFinish(Engine->Window);
    }

    DeleteEngine(Engine);
    return 0;
}

void DrawTerrainTool(engine *Engine,
		     int &mapSize,
		     int mPosX, int mPosY,
		     bool &focus)
{
    ImGui::SetNextWindowPos(ImVec2(10, 160));
    ImGui::SetNextWindowSize(ImVec2(200, 220));
    ImGui::Begin("data", nullptr, ImGuiWindowFlags_NoResize);
    ImGui::Text("Terrain:");
    ImGui::Separator();
    ImGui::SliderInt("range", &mapSize, 0, 100);
    ImGui::Separator();
    ImGui::Text("Picking:");
    ImGui::Separator();
    ImGui::Text("mX= %d / mYinvert %d", mPosX, mPosY);
    if (ImGui::IsWindowFocused())
	focus = true;
    else
	focus = false;
    ImGui::End();
}

void DrawObjectPanel(std::unordered_map<int, entity_cube> &objects, bool &focus)
{
    ImGui::SetNextWindowPos(ImVec2(10, 390));
    ImGui::SetNextWindowSize(ImVec2(420, 300));
    ImGui::Begin("Objects", nullptr, ImGuiWindowFlags_NoResize);

    // left
    static int selected = 0;
    ImGui::BeginChild("left pane", ImVec2(150, 0), true);

    for (std::pair<int, entity_cube> element : objects)	 
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
    ImGui::Text("obj: %d", selectedID);
    ImGui::Separator();
    if (selectedID != 0)
    {
	ImGui::Text("ID: %d", objects[selectedID].ID);
	ImGui::Text("mem: %p", &objects[selectedID]);
	ImGui::Text("State: %s",
		    (objects[selectedID].State == ENTITY_STATIC ? "STATIC" : "DYNAMIC"));
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
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows))
	focus = true;
    else if (!focus)
	focus = false;
    ImGui::End();
}

void CreateTestContainers()
{
    entity_cube containerOne = CreateEntityCube(1,
						{ 3.0f, 0.5f, 0.0f },
						{ 1.0f, 1.0f, 1.0f, 1.0f },
						{ 1.0f, 0.0f, 0.0f, 1.0f });
    entity_cube containerTwo = CreateEntityCube(2,
						{ -3.0f, 0.5f, 0.0f },
						{ 1.0f, 1.0f, 1.0f, 1.0f },
						{ 0.0f, 0.0f, 1.0f, 1.0f });
    
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

void BlackAndWhite(float *r, float *g, float *b) {
    if (*r == 1.0f)
    {
        *r = 0.0f;
        *g = 0.0f;
        *b = 0.0f;
    }
    else if (*r == 0.0f)
    {
        *r = 1.0f;
        *g = 1.0f;
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

void DrawTerrain(renderer *Renderer, int mapSize)
{
    StartNewBatchCube(Renderer);
    float r = 1.0f;
    float g = 1.0f;
    float b = 1.0f;
    float size = 1.0f;
    float posX = 0.0f;
    for (int i = 0; i < mapSize / 2; i++)
    {
	float posZ = -size;
	for (int y = 0; y < mapSize / 2; y++)
	{
	    //CrapColors(&r, &g, &b);
	    BlackAndWhite(&r, &g, &b);
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
	    //CrapColors(&r, &g, &b);
	    BlackAndWhite(&r, &g, &b);
	    AddCubeToBuffer(
		Renderer,
		{ posX, 0.0f, posZ },
		{ size, 0.5f, size, 1.0f },
		{ r, g, b, 1.0f });
	    posZ -= size;
	}
	posX -= size;
    }

    r = 0.0f;
    g = 0.0f;
    b = 0.0f;
    posX = 0.0f;
    for (int i = 0; i < mapSize / 2; i++)
    {
	float posZ = 0.0f;
	for (int y = 0; y < mapSize / 2; y++)
	{
	    //CrapColors(&r, &g, &b);
	    BlackAndWhite(&r, &g, &b);
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
	    //CrapColors(&r, &g, &b);
	    BlackAndWhite(&r, &g, &b);
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
    // transform to clip coord
    glm::vec4 rayClip = glm::vec4(rayNDC.x, rayNDC.y, -1.0, 1.0);
    // transform to camera coord
    glm::vec4 rayEye = inverse(projectionMatrix) * rayClip;
    // manually unproject xy
    rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0, 0.0);
    // transform to world coord
    glm::vec4 stepWorld = inverse(viewMatrix) * rayEye;
    glm::vec3 rayWorld = glm::vec3(stepWorld.x, stepWorld.y, stepWorld.z);
    // don't forget to normalise the vector at some point
    return normalize(rayWorld);
}

bool TestRaySphereIntersection(glm::vec3 rayOriginWorld,
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
