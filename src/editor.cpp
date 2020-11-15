#include "editor.h"
#include "editor_gui.h"

void PrepareAxisDebug(mesh_t *Mesh); // TODO: rename origin
void PushReferenceGridSubData(mesh_t *Mesh, uint32 resolution);
void PushMouseRaySubData(mesh_t *Mesh, glm::vec3 origin, glm::vec3 direction);

// const uint32 g_Width = 1280;
// const uint32 g_Height = 960;

static uint32 g_TerrainResolution = 10;
static const uint32 g_TerrainMaxResolution = 50;
glm::vec3 g_TerrainUnitSize = glm::vec3(0.0f, 0.0f, 0.0f);
const char* g_TerrainDefaultModelFile = "./assets/models/terrain/untitled.obj"; // TODO: read all this kind of stuff from a default config file

// TODO: Clean mess below
static uint32 g_HoveredEntity = 0;
static uint32 g_SelectedEntity = 0;
static uint32 g_DragEntity = 0;
const float32 g_PickingSphereRadius = 0.2f; // Used for draw sphere and ray intersection

const uint32 g_ReferenceGridResolution = 50;

void RunEditorMode(WindowWrapper *Window, InputState *Input)
{
	// window_t *Window = AllocAndInit(g_Width, g_Height, "CrapEngine");
	// input_t *Input = AllocAndInit(Window->PlatformWindow);
     Camera *camera = new Camera((float32)Window->getWidth(), (float32)Window->getHeight(), glm::vec3(0.0f, 5.0f, 10.0f));
	renderer_t *Renderer = AllocAndInit();


    // TODO: parese a list in a dedicated header file
    CompileAndCacheShader("./shaders/default.glsl", "default", camera->projectionMatrix);
    CompileAndCacheShader("./shaders/instanced.glsl", "instanced", camera->projectionMatrix);
    CompileAndCacheShader("./shaders/color.glsl", "color", camera->projectionMatrix);
    CompileAndCacheShader("./shaders/skybox.glsl", "skybox", camera->projectionMatrix);

	// =================================================
	// Grid
	std::vector<uint32> uEmpty;
	std::vector<texture_t> tEmpty;
	std::vector<vertex_t> vGrid(g_ReferenceGridResolution * 4 + 4, vertex_t());
	mesh_t *MeshGrid = AllocAndInit(vGrid, uEmpty, tEmpty);

    // Axis Debug
	std::vector<vertex_t> vAxisDebug(6, vertex_t());
	mesh_t *MeshAxisDebug = AllocAndInit(vAxisDebug, uEmpty, tEmpty);

	// Ray
	std::vector<vertex_t> vRay(2, vertex_t());
	mesh_t *MeshRay = AllocAndInit(vRay, uEmpty, tEmpty);

    // Generate terrain
    terrain_t *Terrain = AllocAndInit(g_TerrainResolution, g_TerrainUnitSize, g_TerrainDefaultModelFile);
    Terrain->ModelMatrices = GenerateTerrainModelMatrices(Terrain->Resolution);

    // Entitys array
	std::map<uint32, entity_t *> *SCENE = new std::map<uint32, entity_t *>;

	// Skybox
	std::vector<std::string> faces{
		"./assets/skybox/test/right.jpg",
		"./assets/skybox/test/left.jpg",
		"./assets/skybox/test/top.jpg",
		"./assets/skybox/test/bottom.jpg",
		"./assets/skybox/test/front.jpg",
		"./assets/skybox/test/back.jpg"};
	// =================================================
    
	editor_t *Editor = new editor_t;
	Editor->Active = true;
	Editor->GridResolution = 0; //TODO: remove?
	Editor->MeshGrid = MeshGrid;
	Editor->MeshAxisDebug = MeshAxisDebug;
	Editor->MeshRay = MeshRay;
	Editor->Skybox = GenerateSkyboxFromFiles(faces);
	Editor->ShowSkybox = false;
     InitEditorGui(Window);

    // TODO: tmp wainting editor struct
    PrepareAxisDebug(Editor->MeshAxisDebug);
    PushReferenceGridSubData(Editor->MeshGrid, g_ReferenceGridResolution);
	// =============================

	while (Editor->Active)
	{
	    Window->updateTime();

		// NOTE: INPUTS ======================================>
	    Window->pollEvents();
		if (Input->Keyboard->isPressed[keyboard::CRAP_KEY_ESCAPE])
			Editor->Active = false;
		if (Input->Keyboard->isPressed[keyboard::CRAP_KEY_W])
		    camera->processMovementDirection(FORWARD, Window->Time->deltaTime);
		if (Input->Keyboard->isPressed[keyboard::CRAP_KEY_S])
		    camera->processMovementDirection(BACKWARD, Window->Time->deltaTime);
		if (Input->Keyboard->isPressed[keyboard::CRAP_KEY_A])
		    camera->processMovementDirection(LEFT, Window->Time->deltaTime);
		if (Input->Keyboard->isPressed[keyboard::CRAP_KEY_D])
		    camera->processMovementDirection(RIGHT, Window->Time->deltaTime);
		if (Input->Keyboard->isPressed[keyboard::CRAP_KEY_SPACE])
		    camera->processMovementDirection(UP, Window->Time->deltaTime);
		if (Input->Keyboard->isPressed[keyboard::CRAP_KEY_LEFT_CONTROL])
		    camera->processMovementDirection(DOWN, Window->Time->deltaTime);

		if (Input->Mouse->scrollOffsetY != 0.0f)
		{
			if (Input->getMouseScrollOffsetY() > 0)
			    camera->processMovementDirection(FORWARD, Window->Time->deltaTime, 10.0f);
			else
			    camera->processMovementDirection(BACKWARD, Window->Time->deltaTime, 10.0f);
		}

		if (Input->Mouse->leftButton)
		{
			if (g_HoveredEntity != 0)
				g_SelectedEntity = g_HoveredEntity;
			else if (g_DragEntity == 0 && !g_ActiveWindow)
				g_SelectedEntity = 0;

			if (g_SelectedEntity != 0 && !g_ActiveWindow)
				g_DragEntity = g_SelectedEntity;

			if (!g_ActiveWindow && !g_SelectedEntity)
            {
                Input->updateMouseOffsets();
                camera->processMovementAngles(Input->Mouse->offsetX, Input->Mouse->offsetY);
            }
		}
		else
            g_DragEntity = 0;

		// NOTE: SIMULATE  ======================================>
		glm::vec3 rayWorld = MouseRayDirectionWorld((float32)Input->Mouse->posX,
                                                    (float32)Input->Mouse->posY,
                                                    Window->getWidth(),
                                                    Window->getHeight(),
                                                    camera->projectionMatrix,
                                                    camera->getViewMatrix());

          // terrain slider
          if (g_TerrainResolution != Terrain->Resolution)
          {
              Terrain->IsGenerated = false;
              Terrain->Resolution = g_TerrainResolution;
          }
        
        // mouse ray intersection sphere selector objects
		for (auto it = SCENE->begin(); it != SCENE->end(); it++)
		{
			float32 rayIntersection = 0.0f;
			glm::vec3 spherePos = glm::vec3(
				it->second->Position.x,
				it->second->Position.y,
				it->second->Position.z);

			if (RaySphereIntersection(camera->position,
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
		if (!RayPlaneIntersection(camera->position,
								  rayWorld, glm::vec3(0.0f),
								  glm::vec3(0.0f, 1.0f, 0.0f),
								  &pIntersection))
			pIntersection = glm::vec3(0.0f);

		// NOTE: RENDERING ======================================>
	    ResetStats(Renderer);
	    NewRenderingContext();
		glm::mat4 viewMatrix = camera->getViewMatrix();

		shader_t *ColorShader = GetShaderFromCache("color");
	    UseProgram(ColorShader);
		SetUniform4fv(ColorShader, "view", viewMatrix);
	    SetUniform4fv(ColorShader, "model", glm::mat4(1.0f));

		// draw reference grid
        SetUniform4f(ColorShader, "color", glm::vec4(0.360f, 0.360f, 0.360f, 1.0f));
        DrawLines(Renderer, Editor->MeshGrid, 1.0f, ColorShader);

        // draw axis debug
        SetUniform4f(ColorShader, "color", glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
        DrawLines(Renderer, Editor->MeshAxisDebug, 2.0f, ColorShader);

        // draw mouse ray
        PushMouseRaySubData(Editor->MeshRay, camera->position, rayWorld);
        SetUniform4f(ColorShader, "color", glm::vec4(1.0f, 0.8f, 0.0f, 1.0));
        DrawLines(Renderer, Editor->MeshRay, 1.0f, ColorShader);

        // draw terrain
        if (!Terrain->IsGenerated)
        {
            CleanInstance(Terrain); // Needed to clear previous matrices pointer and glBuffer
            Terrain->ModelMatrices = GenerateTerrainModelMatrices(Terrain->Resolution);
            Terrain->InstanceBufferID = PrepareInstancedRendering(Terrain->Entity->Model,
                                                                  Terrain->ModelMatrices,
                                                                  Terrain->Resolution * Terrain->Resolution);
            Terrain->IsGenerated = true;
        }
        shader_t *InstancedShader = GetShaderFromCache("instanced");
        UseProgram(InstancedShader);
        SetUniform4fv(InstancedShader, "view", viewMatrix);
        SetUniform4fv(InstancedShader, "model", glm::mat4(1.0f));
        DrawModelInstanced(Renderer,
                           Terrain->Entity->Model,
                           InstancedShader,
                           Terrain->Resolution * Terrain->Resolution);
                  
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
                        Input,
                        camera,
                        Terrain,
                        g_TerrainResolution,
                        g_TerrainMaxResolution,
                        &Editor->ShowSkybox,
                        SCENE,
                        &g_SelectedEntity,
                        g_PickingSphereRadius,
                        g_ActiveWindow);

	    RenderEditorGui();

		// TODO:  update memory pool
		//Renderer->MemoryArena->MaxUsed = 0;

	    Window->swapBuffer();
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
    //delete camera;
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

void PushReferenceGridSubData(mesh_t *Mesh, uint32 resolution)
{
    uint32 vCount = resolution * 4 + 4;			   // 44
    float32 b = (float32)resolution / 2.0f + 1.0f; // 6
    float32 a = -b;								   // -6
    float32 xPos = -((float32)resolution / 2.0f);  // -5
    float32 zPos = xPos;						   // -5

    Mesh->Vertices.clear();
    uint32 i = 0;
    while (i < vCount / 2) // z axis ->
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

    while (i < vCount) // x axis ->
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
