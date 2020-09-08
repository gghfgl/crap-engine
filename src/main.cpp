#include <iostream>

#include "unity_build.h"

/* TODO: EDITOR
   - clean code in services and APIs (editor / engine / render-test)
   - implement memorypool visualizer
   - move skybox and entities in dedicated "Data" panel
   - improve click setting panel object after selectinID from world
   - improve coordinate system for positionning objects
   - improve skybox loading assets with hdr
   - run rendering scene from editor
   - fix / improve texture management from assimp model
   - assimp be carefull of texture path (need to update blender mtl file)
   - change to ortho projection
   - beautyfull light PBR effects
   - improve scene file format (extend with skybox, etc ...)
*/

/* TODO GAME:
   - near / far plan boundary camera
*/

/* TODO Improvments:
   - learn more compiler stuff
   - learn how properly debug with MSVC
   - IMGUI check examples through all versions
*/

// -------------------------------
enum program_mode
{
    EDITOR_MODE,
    RENDER_MODE,
};

// void PushRaySubData(mesh_t *Mesh, glm::vec3 origin, glm::vec3 direction);
// void PushGridSubData(mesh_t *Mesh, uint32 resolution, uint32 maxResolution);
// void PrepareAxisDebug(mesh_t *Mesh);
mesh_t *CreatePickingSphereMesh(float32 margin, float32 radius, uint32 stacks, uint32 slices);
bool RaySphereIntersection(glm::vec3 rayOriginWorld, glm::vec3 rayDirectionWorld, glm::vec3 sphereCenterWorld, float32 sphereRadius, float32 *intersectionDistance);
bool RayPlaneIntersection(glm::vec3 rayOriginWorld, glm::vec3 rayDirectionWorld, glm::vec3 planeCoord, glm::vec3 planeNormal, glm::vec3 *intersectionPoint);
//glm::mat4* GenerateTerrainModelMatrices(uint32 squareSideLenght);

// -------------------------------

struct editor_t
{
	bool Active;
	mesh_t *MeshGrid;
	mesh_t *MeshAxisDebug;
	mesh_t *MeshRay;
	uint32 GridResolution;
	skybox_t *Skybox; // TODO: add skybox load from file in panel with min
	bool ShowSkybox;
};

const uint32 g_Width = 1280;
const uint32 g_Height = 960;
glm::vec3 g_CameraStartPosition = glm::vec3(0.0f, 5.0f, 10.0f);
static uint32 g_CurrentMode = EDITOR_MODE;

// TODO: Editor
static bool g_ActiveWindow = false;
static const uint32 g_GridMaxResolution = 98;
static uint32 g_GridResolutionSlider = 52;
static uint32 g_HoveredEntity = 0;
static uint32 g_SelectedEntity = 0;
static uint32 g_DragEntity = 0;
const float32 g_PickingSphereRadius = 0.2f; // Used for draw sphere and ray intersection
const char* g_TerrainModelFile = "..\\assets\\models\\terrain\\untitled.obj";
static uint32 g_TerrainSideLenght = 10;
static bool g_TerrainPrepared = false; // TODO: tmp wainting terrain dedicated struct

int main(int argc, char *argv[])
{
	window_t *Window = AllocAndInit(g_Width, g_Height, "CrapEngine");
	input_t *InputState = AllocAndInit(Window->PlatformWindow);
	camera_t *Camera = AllocAndInit((float32)g_Width, (float32)g_Height, g_CameraStartPosition);
	renderer_t *Renderer = AllocAndInit();

    CompileAndCacheShader("../shaders/default.glsl", "default", Camera->ProjectionMatrix);
    CompileAndCacheShader("../shaders/instanced.glsl", "instanced", Camera->ProjectionMatrix);
    CompileAndCacheShader("../shaders/color.glsl", "color", Camera->ProjectionMatrix);
    CompileAndCacheShader("../shaders/skybox.glsl", "skybox", Camera->ProjectionMatrix);

	// =================================================
	// Grid
	std::vector<uint32> uEmpty;
	std::vector<texture_t> tEmpty;
	std::vector<vertex_t> vGrid(g_GridMaxResolution * 4, vertex_t());
	mesh_t *MeshGrid = AllocAndInit(vGrid, uEmpty, tEmpty);

    // Axis Debug
	std::vector<vertex_t> vAxisDebug(6, vertex_t());
	mesh_t *MeshAxisDebug = AllocAndInit(vAxisDebug, uEmpty, tEmpty);

	// Ray
	std::vector<vertex_t> vRay(2, vertex_t());
	mesh_t *MeshRay = AllocAndInit(vRay, uEmpty, tEmpty);

    // Generate terrain
    // TODO: Generate from imgui slider?
    model_t *terrainModel = LoadModelFromFile(g_TerrainModelFile);
    glm::mat4 *terrainModelMatrices = GenerateTerrainModelMatrices(g_TerrainSideLenght);

	// Entitys array
	std::map<uint32, entity_t *> *SCENE = new std::map<uint32, entity_t *>;

	// Skybox
	std::vector<std::string> faces{
		"../assets/skybox/test/right.jpg",
		"../assets/skybox/test/left.jpg",
		"../assets/skybox/test/top.jpg",
		"../assets/skybox/test/bottom.jpg",
		"../assets/skybox/test/front.jpg",
		"../assets/skybox/test/back.jpg"};
	// =================================================
    
	editor_t *Editor = new editor_t;
	Editor->Active = true;
	Editor->GridResolution = 0;
	Editor->MeshGrid = MeshGrid;
	Editor->MeshAxisDebug = MeshAxisDebug;
	Editor->MeshRay = MeshRay;
	Editor->Skybox = GenerateSkyboxFromFiles(faces);
	Editor->ShowSkybox = false;
    InitEditorGui(Window);

    // TODO: tmp wainting editor struct
    PrepareAxisDebug(Editor->MeshAxisDebug);

	// =============================

	while (Editor->Active)
	{
	    UpdateTime(Window->Time);

		// NOTE: INPUTS ======================================>
	    PollEvents();
		if (InputState->KeyboardEvent->IsPressed[CRAP_KEY_ESCAPE])
			Editor->Active = false;
		if (InputState->KeyboardEvent->IsPressed[CRAP_KEY_W])
		    ProcessMovementDirectional(Camera, FORWARD, Window->Time->DeltaTime);
		if (InputState->KeyboardEvent->IsPressed[CRAP_KEY_S])
		    ProcessMovementDirectional(Camera, BACKWARD, Window->Time->DeltaTime);
		if (InputState->KeyboardEvent->IsPressed[CRAP_KEY_A])
		    ProcessMovementDirectional(Camera, LEFT, Window->Time->DeltaTime);
		if (InputState->KeyboardEvent->IsPressed[CRAP_KEY_D])
		    ProcessMovementDirectional(Camera, RIGHT, Window->Time->DeltaTime);
		if (InputState->KeyboardEvent->IsPressed[CRAP_KEY_SPACE])
		    ProcessMovementDirectional(Camera, UP, Window->Time->DeltaTime);
		if (InputState->KeyboardEvent->IsPressed[CRAP_KEY_LEFT_CONTROL])
		    ProcessMovementDirectional(Camera, DOWN, Window->Time->DeltaTime);

		if (InputState->MouseEvent->ScrollOffsetY != 0.0f)
		{
			if (GetMouseScrollOffsetY(InputState->MouseEvent) > 0)
			    ProcessMovementDirectional(Camera, FORWARD,
												   Window->Time->DeltaTime, 10.0f);
			else
			    ProcessMovementDirectional(Camera, BACKWARD,
												   Window->Time->DeltaTime, 10.0f);
		}

		if (InputState->MouseEvent->LeftButton)
		{
			if (g_HoveredEntity != 0)
				g_SelectedEntity = g_HoveredEntity;
			else if (g_DragEntity == 0 && !g_ActiveWindow)
				g_SelectedEntity = 0;

			if (g_SelectedEntity != 0 && !g_ActiveWindow)
				g_DragEntity = g_SelectedEntity;

			if (!g_ActiveWindow && !g_SelectedEntity)
            {
                UpdateMouseOffsets(InputState->MouseEvent);
                ProcessMovementAngles(Camera,
                                              InputState->MouseEvent->OffsetX,
                                              InputState->MouseEvent->OffsetY);
            }
		}
		else
            g_DragEntity = 0;

		// NOTE: SIMULATE  ======================================>
		glm::vec3 rayWorld = MouseRayDirectionWorld((float32)InputState->MouseEvent->PosX,
                                                           (float32)InputState->MouseEvent->PosY,
                                                           Window->Width,
                                                           Window->Height,
                                                           Camera->ProjectionMatrix,
                                                           GetViewMatrix(Camera));

        // TODO: mouse ray intersection terrain cubes
          
        // mouse ray intersection sphere selector objects
		for (auto it = SCENE->begin(); it != SCENE->end(); it++)
		{
			float32 rayIntersection = 0.0f;
			glm::vec3 spherePos = glm::vec3(
				it->second->Position.x,
				it->second->Position.y,
				it->second->Position.z);

			if (RaySphereIntersection(Camera->Position,
									  rayWorld,
									  spherePos,
									  g_PickingSphereRadius,
									  &rayIntersection))
			{
				g_HoveredEntity = it->first;
				break;
			}
			else
				g_HoveredEntity = 0;
		}
          
		glm::vec3 pIntersection = glm::vec3(0.0f);
		if (!RayPlaneIntersection(Camera->Position,
								  rayWorld, glm::vec3(0.0f),
								  glm::vec3(0.0f, 1.0f, 0.0f),
								  &pIntersection))
			pIntersection = glm::vec3(0.0f);

		// NOTE: RENDERING ======================================>
	    ResetStats(Renderer);
	    NewRenderingContext(Renderer);
		glm::mat4 viewMatrix = GetViewMatrix(Camera);

		shader_t *ColorShader = GetShaderFromCache("color");
	    UseProgram(ColorShader);
		SetUniform4fv(ColorShader, "view", viewMatrix);
	    SetUniform4fv(ColorShader, "model", glm::mat4(1.0f));

        // =================== D.E.B.U.G ===================
        
		// draw editor grid
		if (Editor->GridResolution != g_GridResolutionSlider)
		{
			PushGridSubData(Editor->MeshGrid, g_GridResolutionSlider, g_GridMaxResolution);
			Editor->GridResolution = g_GridResolutionSlider;
		}
		if (Editor->GridResolution > 0)
		{
		    SetUniform4f(ColorShader, "color", glm::vec4(0.360f, 0.360f, 0.360f, 1.0f));
		    DrawLines(Renderer, Editor->MeshGrid, 1.0f, ColorShader);
		}

        // draw axis debug
        SetUniform4f(ColorShader, "color", glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
        DrawLines(Renderer, MeshAxisDebug, 2.0f, ColorShader);

		// draw raycasting
	    PushMouseRaySubData(Editor->MeshRay, Camera->Position, rayWorld);
	    SetUniform4f(ColorShader, "color", glm::vec4(1.0f, 0.8f, 0.0f, 1.0));
	    DrawLines(Renderer, Editor->MeshRay, 1.0f, ColorShader);

        // =================== M.O.D.E.L.S ===================

        // draw terrain
        if (!g_TerrainPrepared)
        {
            PrepareInstancedRendering(Renderer,
                                                terrainModel,
                                                terrainModelMatrices,
                                                g_TerrainSideLenght * g_TerrainSideLenght);
            g_TerrainPrepared = true;
        }
		shader_t *InstancedShader = GetShaderFromCache("instanced");
	    UseProgram(InstancedShader);
	    SetUniform4fv(InstancedShader, "view", viewMatrix);
	    SetUniform4fv(InstancedShader, "model", glm::mat4(1.0f));
        DrawModelInstanced(Renderer,
                                     terrainModel,
                                     InstancedShader,
                                     g_TerrainSideLenght * g_TerrainSideLenght);
        
                  
		// draw objs
		shader_t *DefaultShader = GetShaderFromCache("default");
	    UseProgram(DefaultShader);
	    SetUniform4fv(DefaultShader, "view", viewMatrix);
	    SetUniform4fv(DefaultShader, "model", glm::mat4(1.0f));
		for (auto it = SCENE->begin(); it != SCENE->end(); it++)
		{
			bool isSelected = false;
			if (g_HoveredEntity == it->first || g_SelectedEntity == it->first)
				isSelected = true;

			if (g_DragEntity == it->first)
				it->second->Position = pIntersection;

			// Model
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, it->second->Position);
			model = glm::scale(model, glm::vec3(it->second->Scale));
			model = glm::rotate(model, glm::radians(it->second->Rotate),
								glm::vec3(0.0f, 1.0f, 0.0f));
		    SetUniform4fv(DefaultShader, "model", model);
		    SetUniform1ui(DefaultShader, "flip_color", isSelected);
		    DrawModel(Renderer, it->second->Model, DefaultShader);

			// Picking sphere
			model = glm::mat4(1.0f);
			model = glm::translate(model, it->second->Position);
		    SetUniform4fv(DefaultShader, "model", model);
		    SetUniform1ui(DefaultShader, "flip_color", isSelected);
		    DrawMesh(Renderer, it->second->PickingSphere, DefaultShader);
		}

        // =================== E.N.V.I.R.O.N.M.E.N.T ===================

		// Skybox
		if (Editor->Skybox != NULL && Editor->ShowSkybox)
		{
			shader_t *SkyboxShader = GetShaderFromCache("skybox");
		    UseProgram(SkyboxShader);
		    SetUniform4fv(SkyboxShader, "view", glm::mat4(glm::mat3(viewMatrix))); // remove translation from the view matrix
		    DrawSkybox(Renderer, Editor->Skybox, SkyboxShader);
		}

        // =================== G.U.I ===================

		// draw GUI
	    NewFrameEditorGui();
	    ShowWindowStatsOverlay(Window, Renderer);
	    ShowEditorPanel(Window,
								   InputState,
								   Camera,
								   g_GridResolutionSlider,
								   g_GridMaxResolution,
								   &Editor->ShowSkybox,
								   SCENE,
								   &g_SelectedEntity,
								   g_PickingSphereRadius,
								   g_ActiveWindow);

	    RenderEditorGui();

		// TODO:  update memory pool
		//Renderer->MemoryArena->MaxUsed = 0;

	    SwapBuffer(Window);
	}

    delete []terrainModelMatrices;
	for (auto it = SCENE->begin(); it != SCENE->end(); it++)
	    Delete(it->second);
	delete SCENE;
    // TODO: implement complete full delete Editor method
    Delete(Editor->MeshGrid);
    Delete(Editor->MeshAxisDebug);
    Delete(Editor->MeshRay);
    Delete(Editor->Skybox);
	delete Editor;

    DeleteEditorGui();
    ClearShaderCache();
    Delete(Camera);
    Delete(Renderer);
    Delete(InputState);
    Delete(Window);

// ============================================
    // TODO
    switch(g_CurrentMode)
    {
    case EDITOR_MODE:
        // encapsulate editor code
        // create a world saved in file
        break;
    case RENDER_MODE:
        // encapsulte game code
        // Render a world from file
        break;
    default:
        // here ...
        break;
    };
// ============================================
    
	return 0;
}
