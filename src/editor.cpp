#include <iostream>

#include "Engine/unity_build.h"
#include "editor.h"
#include "editor_gui.h"

/* TODO: EDITOR
   - working on terrain editor
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

void PrepareAxisDebug(mesh_t *Mesh);
void PushGridSubData(mesh_t *Mesh, uint32 resolution, uint32 maxResolution);
void PushMouseRaySubData(mesh_t *Mesh, glm::vec3 origin, glm::vec3 direction);

const uint32 g_Width = 1280;
const uint32 g_Height = 960;

static const uint32 g_GridMaxResolution = 98;
static uint32 g_GridResolutionSlider = 52;
static uint32 g_HoveredEntity = 0;
static uint32 g_SelectedEntity = 0;
static uint32 g_DragEntity = 0;
const float32 g_PickingSphereRadius = 0.2f; // Used for draw sphere and ray intersection

const char* g_TerrainModelFile = "..\\assets\\models\\terrain\\untitled.obj";

void RunEditorMode(uint32 currentMode)
{
	window_t *Window = AllocAndInit(g_Width, g_Height, "CrapEngine");
	input_t *InputState = AllocAndInit(Window->PlatformWindow);
	camera_t *Camera = AllocAndInit((float32)g_Width, (float32)g_Height, glm::vec3(0.0f, 5.0f, 10.0f));
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
    terrain_t *Terrain = AllocAndInit(g_TerrainModelFile);

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
        if (!Terrain->IsGenerated)
        {
            PrepareInstancedRendering(Renderer,
                                      Terrain->Entity->Model,
                                      Terrain->ModelMatrices,
                                      Terrain->SideLenght * Terrain->SideLenght);
            Terrain->IsGenerated = true;
        }
		shader_t *InstancedShader = GetShaderFromCache("instanced");
	    UseProgram(InstancedShader);
	    SetUniform4fv(InstancedShader, "view", viewMatrix);
	    SetUniform4fv(InstancedShader, "model", glm::mat4(1.0f));
        DrawModelInstanced(Renderer,
                           Terrain->Entity->Model,
                           InstancedShader,
                           Terrain->SideLenght * Terrain->SideLenght);
        
                  
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

	for (auto it = SCENE->begin(); it != SCENE->end(); it++)
	    Delete(it->second);
	delete SCENE;
    Delete(Terrain);
    
    // TODO: implement complete full delete Editor method
    Delete(Editor->MeshGrid);
    Delete(Editor->MeshAxisDebug);
    Delete(Editor->MeshRay);
    Delete(Editor->Skybox);
	delete Editor;

    DeleteEditorGui();
    ClearShaderCache();

    Delete(Renderer);
    Delete(Camera);
    Delete(InputState);
    Delete(Window);
}
void PrepareAxisDebug(mesh_t *Mesh)
{    
    Mesh->Vertices.clear();

    vertex_t vXa;
    vXa.Position = glm::vec3(0.0f, 0.1f, 0.0f);
    Mesh->Vertices.push_back(vXa);
    vertex_t vXb;
    vXb.Position = glm::vec3(2.0f, 0.1f, 0.0f);
    Mesh->Vertices.push_back(vXb);

    vertex_t vYa;
    vYa.Position = glm::vec3(0.0f, 0.1f, 0.0f);
    Mesh->Vertices.push_back(vYa);
    vertex_t vYb;
    vYb.Position = glm::vec3(0.0f, 2.0f, 0.0f);
    Mesh->Vertices.push_back(vYb);

    vertex_t vZa;
    vZa.Position = glm::vec3(0.0f, 0.1f, 0.0f);
    Mesh->Vertices.push_back(vZa);
    vertex_t vZb;
    vZb.Position = glm::vec3(0.0f, 0.1f, -2.0f);
    Mesh->Vertices.push_back(vZb);
    
    glBindBuffer(GL_ARRAY_BUFFER, Mesh->VBO);
    glBufferSubData(GL_ARRAY_BUFFER,
                    0,
                    Mesh->Vertices.size() * sizeof(vertex_t),
                    &Mesh->Vertices[0]);
}

void PushGridSubData(mesh_t *Mesh, uint32 resolution, uint32 maxResolution)
{
	if (resolution <= maxResolution)
	{
		uint32 vCount = resolution * 4 + 4;			   // 44
		float32 b = (float32)resolution / 2.0f + 1.0f; // 6
		float32 a = -b;								   // -6
		float32 xPos = -((float32)resolution / 2.0f);  // -5
		float32 zPos = xPos;						   // -5

		Mesh->Vertices.clear();
		uint32 i = 0;
		while (i < vCount / 2)
		{
			vertex_t v;
			if (i % 2 == 0)
			{
				v.Position = glm::vec3(a, 0.0f, zPos);
			}
			else
			{
				v.Position = glm::vec3(b, 0.0f, zPos);
				zPos += 1.0f;
			}

			Mesh->Vertices.push_back(v);
			i++;
		}

		while (i < vCount)
		{
			vertex_t v;
			if (i % 2 == 0)
			{
				v.Position = glm::vec3(xPos, 0.0f, a);
			}
			else
			{
				v.Position = glm::vec3(xPos, 0.0f, b);
				xPos += 1.0f;
			}

			Mesh->Vertices.push_back(v);
			i++;
		}
        
		glBindBuffer(GL_ARRAY_BUFFER, Mesh->VBO);
		glBufferSubData(GL_ARRAY_BUFFER,
						0,
						Mesh->Vertices.size() * sizeof(vertex_t),
						&Mesh->Vertices[0]);
	}
}

void PushMouseRaySubData(mesh_t *Mesh, glm::vec3 origin, glm::vec3 direction)
{
	glm::vec3 target = origin + (direction * 1.0f);

	Mesh->Vertices.clear();
	vertex_t v;
	v.Position = glm::vec3(origin.x, origin.y, origin.z - 0.1f);
	Mesh->Vertices.push_back(v);
	v.Position = target;
	Mesh->Vertices.push_back(v);

	glBindBuffer(GL_ARRAY_BUFFER, Mesh->VBO);
	glBufferSubData(GL_ARRAY_BUFFER,
					0,
					Mesh->Vertices.size() * sizeof(vertex_t),
					&Mesh->Vertices[0]);
}
