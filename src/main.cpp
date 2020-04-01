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
*/

/* TODO:
   - stencil test outline object + picking object by ray casting (CPU)
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
BYTE bArray[4];
int iResult = 0;
// entity_cube selectedObject;
int selectedID = 0;
//std::vector<entity_cube> Containers; // TODO: NEXT STEP HERE!!!!! Fill a cube vector 
std::unordered_map<int, entity_cube> CONTAINER_ENTITIES;

glm::vec4 GetColorByIndex(int index);
void CrapColors(float *r, float *g, float *b);
void DrawTerrain(renderer *Renderer, int mapSize);
void DrawCubeContainer(renderer *Renderer, entity_cube container, float scale);
void CreateTestContainers();
void DrawTerrainTool(engine *Engine, int &mapSize, int mPosX, int mPosY, BYTE pickRGB[], int pickValue, bool &focus);
void DrawObjectPanel(std::unordered_map<int, entity_cube> &objects, bool &focus);
void ScreenPosToWorldRay(int mouseX, int mouseY, int screenWidth, int screenHeight, glm::mat4 ViewMatrix, glm::mat4 ProjectionMatrix, glm::vec3& out_origin, glm::vec3& out_direction);
bool TestRayOBBIntersection(glm::vec3 ray_origin, glm::vec3 ray_direction, glm::vec3 aabb_min, glm::vec3 aabb_max, glm::mat4 ModelMatrix, float& intersection_distance);

int main(int argc, char *argv[])
{
    engine *Engine = EngineConstructAndInit(1280, 960, DEBUG_MODE | VSYNC);
    PrepareEmbededAxisDebugRendering(Engine->Renderer);
    PrepareCubeBatchRendering(Engine->Renderer);

    // TODO: =========================================
    unsigned int TestVAO, TestVBO;
    glGenVertexArrays(1, &TestVAO);
    glBindVertexArray(TestVAO);

    glGenBuffers(1, &TestVBO);
    glBindBuffer(GL_ARRAY_BUFFER, TestVBO);
    glBufferData(GL_ARRAY_BUFFER, 14 * sizeof(float), nullptr, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
    // ================================================
    
    CreateTestContainers(); // quick way to create stuff

    while (Engine->GlobalState == ENGINE_ACTIVE)
    {
	// color picking
	int invertMouseY = Engine->Height - (int)Engine->InputState->MousePosY;
	glReadPixels((int)Engine->InputState->MousePosX, invertMouseY,
		     1, 1, GL_RGB, GL_UNSIGNED_BYTE, bArray);
	iResult = GetIndexByColor(bArray[0], bArray[1], bArray[2]);

	// // mouse ray casting
	// // transform to NDC
	// float mx = (2.0f * (float)Engine->InputState->MousePosX) / Engine->Width - 1.0f;
	// float my = 1.0f - (2.0f * (float)Engine->InputState->MousePosY) / Engine->Height;
	// float mz = 1.0f;
	// glm::vec3 ray_nds = glm::vec3(mx, my, mz);
	// // transform to clip coord
	// glm::vec4 ray_clip = glm::vec4(ray_nds.x, ray_nds.y, -1.0, 1.0);
	// // transform to camera coord
	// glm::vec4 ray_eye = inverse(Engine->ProjMatrix) * ray_clip;
	// // manually unproject xy
	// ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);
	// // transform to world coord
	// glm::vec4 test = inverse(GetCameraViewMatrix(Engine->Camera)) * ray_eye;
        // glm::vec3 ray_world = glm::vec3(test.x, test.y, test.z);
        // // don't forget to normalise the vector at some point
	// ray_world = normalize(ray_world);

	//// ====
	
	glm::vec3 ray_origin;
	glm::vec3 ray_direction;
	ScreenPosToWorldRay(
	    (int)Engine->InputState->MousePosX, invertMouseY,
	    Engine->Width, Engine->Height, 
	    GetCameraViewMatrix(Engine->Camera), 
	    Engine->ProjMatrix, 
	    ray_origin, 
	    ray_direction);
	// std::cout << "x=" << ray_direction.x
	// 	  << " y=" << ray_direction.y
	// 	  << " z=" << ray_direction.z
	// 	  << std::endl;

	for (std::pair<int, entity_cube> element : CONTAINER_ENTITIES)
	{
	    float intersection_distance; // Output of TestRayOBBIntersection()
	    glm::vec3 aabb_min(-1.0f, -1.0f, -1.0f);
	    glm::vec3 aabb_max( 1.0f,  1.0f,  1.0f);

	    // The ModelMatrix transforms :
	    // - the mesh to its desired position and orientation
	    // - but also the AABB (defined with aabb_min and aabb_max) into an OBB
	    // glm::mat4 RotationMatrix = glm::toMat4(orientations[i]);
	    glm::mat4 model = glm::mat4(1.0f);
	    glm::mat4 TranslationMatrix = glm::translate(model, element.second.Position);
	    glm::mat4 ModelMatrix = TranslationMatrix;

	    if (TestRayOBBIntersection(
		    ray_origin, 
		    ray_direction, 
		    aabb_min, 
		    aabb_max,
		    ModelMatrix,
		    intersection_distance))
	    {
		std::cout << "x=" << ray_direction.x
			  << " y=" << ray_direction.y
			  << " z=" << ray_direction.z
			  << std::endl;
	    }
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

	    if (CONTAINER_ENTITIES.find(iResult) != CONTAINER_ENTITIES.end())
		selectedID = CONTAINER_ENTITIES[iResult].ID;
	    else
	        selectedID = 0;

	}

	// NOTE: START RENDERING ======================================
	ResetRendererStats(Engine->Renderer);
        StartRendering(Engine->Renderer, Engine->Camera);

	// TODO: debug ray casting
	float test_ray[] =
	    {
		ray_origin.x, ray_origin.y, ray_origin.z, 1.0f, 0.0f, 0.0f, 1.0f,
		ray_direction.x, ray_direction.y, ray_direction.z, 1.0f, 0.0f, 0.0f, 1.0f,
	    };

	glBindBuffer(GL_ARRAY_BUFFER, TestVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(test_ray), test_ray);
	glBindVertexArray(TestVAO);
	glDrawArrays(GL_LINES, 0, 2);
	//Renderer->Stats.DrawCount++;

	// NOTE: CLASSIC RENDERING ======================================
	// debug tools
	if (Engine->DebugMode)
	    DrawAxisDebug(Engine->Renderer);	

	DrawTerrain(Engine->Renderer, MAPSIZE);

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

	if (CONTAINER_ENTITIES.find(iResult) != CONTAINER_ENTITIES.end() || selectedID != 0)
	{
	    StartStencilRendering(Engine->Renderer, Engine->Camera);

	    StartNewBatchCube(Engine->Renderer);

	    if (selectedID != 0)
		DrawCubeContainer(Engine->Renderer, CONTAINER_ENTITIES[selectedID], 1.1f);
	    if (CONTAINER_ENTITIES.find(iResult) != CONTAINER_ENTITIES.end() && iResult != selectedID)
		DrawCubeContainer(Engine->Renderer, CONTAINER_ENTITIES[iResult], 1.1f);
	    CloseBatchCube(Engine->Renderer);
	    FlushBatchCube(Engine->Renderer);

	    StopRenderingStencil();	    
	}

	// NOTE: UI RENDERING ======================================
	StartImGuiRendering();	

	DrawDebugOverlay(Engine);
        DrawTerrainTool(Engine,
			MAPSIZE,
			(int)Engine->InputState->MousePosX, invertMouseY,
			bArray, iResult,
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
		     BYTE pickRGB[], int pickValue,
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
    ImGui::Text("RGB=%d,%d,%d", pickRGB[0], pickRGB[1], pickRGB[2]);
    ImGui::Text("bitValue= %d", pickValue);
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
    ImGui::Text("obj: %d", selected);
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
    entity_cube containerOne = CreateEntityCube({ 3.0f, 0.5f, 0.0f },
						{ 1.0f, 1.0f, 1.0f, 1.0f },
						{ 1.0f, 0.0f, 0.0f, 1.0f });
    entity_cube containerTwo = CreateEntityCube({ -3.0f, 0.5f, 0.0f },
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

void ScreenPosToWorldRay(
    int mouseX, int mouseY,             // Mouse position, in pixels, from bottom-left corner of the window
    int screenWidth, int screenHeight,  // Window size, in pixels
    glm::mat4 ViewMatrix,               // Camera position and orientation
    glm::mat4 ProjectionMatrix,         // Camera parameters (ratio, field of view, near and far planes)
    glm::vec3& out_origin,              // Ouput : Origin of the ray. /!\ Starts at the near plane, so if you want the ray to start at the camera's position instead, ignore this.
    glm::vec3& out_direction            // Ouput : Direction, in world space, of the ray that goes "through" the mouse.
    ){
    // The ray Start and End positions, in Normalized Device Coordinates (Have you read Tutorial 4 ?)
    glm::vec4 lRayStart_NDC(
	(2.0f * (float)mouseX) / (float)screenWidth - 1.0f,
	(2.0f * (float)mouseY) / (float)screenHeight - 1.0f,
	-1.0f, // The near plane maps to Z=-1 in Normalized Device Coordinates
	1.0f);

    glm::vec4 lRayEnd_NDC(
	(2.0f * (float)mouseX) / (float)screenWidth - 1.0f,
	(2.0f * (float)mouseY) / (float)screenHeight - 1.0f,
	0.0f,
	1.0f);

    // The Projection matrix goes from Camera Space to NDC.
    // So inverse(ProjectionMatrix) goes from NDC to Camera Space.
    glm::mat4 InverseProjectionMatrix = glm::inverse(ProjectionMatrix);
	
    // The View Matrix goes from World Space to Camera Space.
    // So inverse(ViewMatrix) goes from Camera Space to World Space.
    glm::mat4 InverseViewMatrix = glm::inverse(ViewMatrix);
	
    // glm::vec4 lRayStart_camera = InverseProjectionMatrix * lRayStart_NDC;    lRayStart_camera/=lRayStart_camera.w;
    // glm::vec4 lRayStart_world  = InverseViewMatrix       * lRayStart_camera; lRayStart_world /=lRayStart_world .w;
    // glm::vec4 lRayEnd_camera   = InverseProjectionMatrix * lRayEnd_NDC;      lRayEnd_camera  /=lRayEnd_camera  .w;
    // glm::vec4 lRayEnd_world    = InverseViewMatrix       * lRayEnd_camera;   lRayEnd_world   /=lRayEnd_world   .w;

    // Faster way (just one inverse)
    glm::mat4 M = glm::inverse(ProjectionMatrix * ViewMatrix);
    glm::vec4 lRayStart_world = M * lRayStart_NDC; lRayStart_world/=lRayStart_world.w;
    glm::vec4 lRayEnd_world   = M * lRayEnd_NDC  ; lRayEnd_world  /=lRayEnd_world.w;

    glm::vec3 lRayDir_world(lRayEnd_world - lRayStart_world);
    lRayDir_world = glm::normalize(lRayDir_world);

    out_origin = glm::vec3(lRayStart_world);
    out_direction = glm::normalize(lRayDir_world);
}

bool TestRayOBBIntersection(
	glm::vec3 ray_origin,        // Ray origin, in world space
	glm::vec3 ray_direction,     // Ray direction (NOT target position!), in world space. Must be normalize()'d.
	glm::vec3 aabb_min,          // Minimum X,Y,Z coords of the mesh when not transformed at all.
	glm::vec3 aabb_max,          // Maximum X,Y,Z coords. Often aabb_min*-1 if your mesh is centered, but it's not always the case.
	glm::mat4 ModelMatrix,       // Transformation applied to the mesh (which will thus be also applied to its bounding box)
	float& intersection_distance // Output : distance between ray_origin and the intersection with the OBB
    ){	

    // std::cout << "x= " << ModelMatrix[3].x
    // 	      << " y= " << ModelMatrix[3].y
    // 	      << " z= " << ModelMatrix[3].z
    // 	      << std::endl;

// Intersection method from Real-Time Rendering and Essential Mathematics for Games	
    float tMin = 0.0f;
    float tMax = 100000.0f;
    glm::vec3 OBBposition_worldspace(ModelMatrix[3].x, ModelMatrix[3].y, ModelMatrix[3].z);
    glm::vec3 delta = OBBposition_worldspace - ray_origin;

    // Test intersection with the 2 planes perpendicular to the OBB's X axis
    {
	glm::vec3 xaxis(ModelMatrix[0].x, ModelMatrix[0].y, ModelMatrix[0].z);
	float e = glm::dot(xaxis, delta);
	float f = glm::dot(ray_direction, xaxis);

	if ( fabs(f) > 0.001f ){ // Standard case

	    float t1 = (e+aabb_min.x)/f; // Intersection with the "left" plane
	    float t2 = (e+aabb_max.x)/f; // Intersection with the "right" plane
	    // t1 and t2 now contain distances betwen ray origin and ray-plane intersections
	    
	    // We want t1 to represent the nearest intersection, 
	    // so if it's not the case, invert t1 and t2
	    if (t1>t2){
		float w=t1;t1=t2;t2=w; // swap t1 and t2
	    }

	    // tMax is the nearest "far" intersection (amongst the X,Y and Z planes pairs)
	    if ( t2 < tMax )
		tMax = t2;
	    // tMin is the farthest "near" intersection (amongst the X,Y and Z planes pairs)
	    if ( t1 > tMin )
		tMin = t1;

	    // And here's the trick :
	    // If "far" is closer than "near", then there is NO intersection.
	    // See the images in the tutorials for the visual explanation.
	    if (tMax < tMin )
		return false;

	}else{ // Rare case : the ray is almost parallel to the planes, so they don't have any "intersection"
	    if(-e+aabb_min.x > 0.0f || -e+aabb_max.x < 0.0f)
		return false;
	}
    }

    // Test intersection with the 2 planes perpendicular to the OBB's Y axis
    // Exactly the same thing than above.
    {
	glm::vec3 yaxis(ModelMatrix[1].x, ModelMatrix[1].y, ModelMatrix[1].z);
	float e = glm::dot(yaxis, delta);
	float f = glm::dot(ray_direction, yaxis);

	if ( fabs(f) > 0.001f ){

	    float t1 = (e+aabb_min.y)/f;
	    float t2 = (e+aabb_max.y)/f;

	    if (t1>t2){float w=t1;t1=t2;t2=w;}

	    if ( t2 < tMax )
		tMax = t2;
	    if ( t1 > tMin )
		tMin = t1;
	    if (tMin > tMax)
		return false;

	}else{
	    if(-e+aabb_min.y > 0.0f || -e+aabb_max.y < 0.0f)
		return false;
	}
    }

    // Test intersection with the 2 planes perpendicular to the OBB's Z axis
    // Exactly the same thing than above.
    {
	glm::vec3 zaxis(ModelMatrix[2].x, ModelMatrix[2].y, ModelMatrix[2].z);
	float e = glm::dot(zaxis, delta);
	float f = glm::dot(ray_direction, zaxis);

	if ( fabs(f) > 0.001f ){

	    float t1 = (e+aabb_min.z)/f;
	    float t2 = (e+aabb_max.z)/f;

	    if (t1>t2){float w=t1;t1=t2;t2=w;}

	    if ( t2 < tMax )
		tMax = t2;
	    if ( t1 > tMin )
		tMin = t1;
	    if (tMin > tMax)
		return false;

	}else{
	    if(-e+aabb_min.z > 0.0f || -e+aabb_max.z < 0.0f)
		return false;
	}
    }

    intersection_distance = tMin;
    return true;
}
