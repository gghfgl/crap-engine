#include <iostream>

#include "plateform.h"

/* TODO:
   - infinite grid in editor? with shader instead of cpp?
   - clean shaders
   - implement memorypool visualizer
   - improve coordinate system for positionning objects
   - improve skybox loading assets with hdr
   - clean code in services and APIs (editor / engine / gameplay)
   - run rendering scene from editor
   - fix / improve texture management from assimp model
   - change to ortho projection
   - beautyfull light PBR effects

   - learn more compiler stuff
   - learn how properly debug with MSVC
   - IMGUI check examples through all versions
*/

/* TODO Improvments:
   - assimp be carefull of texture path (need to update blender mtl file)
   - improve click setting panel object after selectinID from world
   - improve grid rendering
   - improve scene file format (extend with skybox, etc ...)
*/

// -------------------------------

void PushRaySubData(mesh_t *Mesh, glm::vec3 origin, glm::vec3 direction);
void PushGridSubData(mesh_t *Mesh, uint32 resolution, uint32 maxResolution);
void PrepareAxisDebug(mesh_t *Mesh);
mesh_t *CreatePickingSphereMesh(float32 margin, float32 radius, uint32 stacks, uint32 slices);
bool RaySphereIntersection(glm::vec3 rayOriginWorld, glm::vec3 rayDirectionWorld, glm::vec3 sphereCenterWorld, float32 sphereRadius, float32 *intersectionDistance);
bool RayPlaneIntersection(glm::vec3 rayOriginWorld, glm::vec3 rayDirectionWorld, glm::vec3 planeCoord, glm::vec3 planeNormal, glm::vec3 *intersectionPoint);
glm::mat4* GenerateTerrainModelMatrices(uint32 squareSideLenght);

// -------------------------------

struct editor_t
{
	bool Active;
	mesh_t *MeshGrid;
	mesh_t *MeshAxisDebug;
	mesh_t *MeshRay;
	uint32 GridResolution;
	skybox_t *Skybox;
	bool ShowSkybox;
};

const uint32 g_Width = 1280;
const uint32 g_Height = 960;
static bool g_ActiveWindow = false;
glm::vec3 g_CameraStartPosition = glm::vec3(0.0f, 5.0f, 10.0f);
static const uint32 g_GridMaxResolution = 52;
static uint32 g_GridResolutionSlider = 10;
const char* g_TerrainModelFile = "..\\assets\\models\\terrain\\untitled.obj";
static uint32 g_TerrainSideLenght = 10;
static bool g_TerrainPrepared = false; // TODO: tmp wainting terrain dedicated struct
static uint32 g_HoveredObject = 0;
static uint32 g_SelectedObject = 0;
static uint32 g_DragObject = 0;
const float32 g_PickingSphereRadius = 0.2f; // Used for draw sphere and ray intersection

int main(int argc, char *argv[])
{
	window_t *Window = window::Construct(g_Width, g_Height, "CrapEngine");
	input_t *InputState = input::Construct(Window->PlatformWindow);
	camera_t *Camera = camera::Construct((float32)g_Width, (float32)g_Height, g_CameraStartPosition);
	renderer_t *Renderer = renderer::Construct();

	shader::CompileAndCache("../shaders/default.glsl", "default", Camera->ProjectionMatrix);
	shader::CompileAndCache("../shaders/instanced.glsl", "instanced", Camera->ProjectionMatrix);
	shader::CompileAndCache("../shaders/color.glsl", "color", Camera->ProjectionMatrix);
	shader::CompileAndCache("../shaders/skybox.glsl", "skybox", Camera->ProjectionMatrix);

	// =================================================
	// Grid
	std::vector<uint32> uEmpty;
	std::vector<texture_t> tEmpty;
	std::vector<vertex_t> vGrid(g_GridMaxResolution * 4, vertex_t());
	mesh_t *MeshGrid = mesh::Construct(vGrid, uEmpty, tEmpty);

    // Axis Debug
	std::vector<vertex_t> vAxisDebug(6, vertex_t());
	mesh_t *MeshAxisDebug = mesh::Construct(vAxisDebug, uEmpty, tEmpty);

	// Ray
	std::vector<vertex_t> vRay(2, vertex_t());
	mesh_t *MeshRay = mesh::Construct(vRay, uEmpty, tEmpty);

    // Generate terrain
    // TODO: Generate from imgui slider?
    model_t *terrainModel = model::LoadFromFile(g_TerrainModelFile);
    glm::mat4 *terrainModelMatrices = GenerateTerrainModelMatrices(g_TerrainSideLenght);

	// Objects array
	std::map<uint32, object_t *> *SCENE = new std::map<uint32, object_t *>;

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
	Editor->Skybox = skybox::GenerateFromFiles(faces);
	Editor->ShowSkybox = false;
	editorGUI::Init(Window);

    // TODO: tmp wainting editor struct
    PrepareAxisDebug(Editor->MeshAxisDebug);

	// =============================

	while (Editor->Active)
	{
		window::UpdateTime(Window->Time);

		// NOTE: INPUTS ======================================>
		input::PollEvents();
		if (InputState->KeyboardEvent->IsPressed[CRAP_KEY_ESCAPE])
			Editor->Active = false;
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

		if (InputState->MouseEvent->ScrollOffsetY != 0.0f)
		{
			if (input::GetMouseScrollOffsetY(InputState->MouseEvent) > 0)
				camera::ProcessMovementDirectional(Camera, FORWARD,
												   Window->Time->DeltaTime, 10.0f);
			else
				camera::ProcessMovementDirectional(Camera, BACKWARD,
												   Window->Time->DeltaTime, 10.0f);
		}

		if (InputState->MouseEvent->LeftButton)
		{
			if (g_HoveredObject != 0)
				g_SelectedObject = g_HoveredObject;
			else if (g_DragObject == 0 && !g_ActiveWindow)
				g_SelectedObject = 0;

			if (g_SelectedObject != 0 && !g_ActiveWindow)
				g_DragObject = g_SelectedObject;

			if (!g_ActiveWindow && !g_SelectedObject)
            {
                input::UpdateMouseOffsets(InputState->MouseEvent);
                camera::ProcessMovementAngles(Camera,
                                              InputState->MouseEvent->OffsetX,
                                              InputState->MouseEvent->OffsetY);
            }
		}
		else
            g_DragObject = 0;

		// NOTE: SIMULATE  ======================================>
		glm::vec3 rayWorld = input::MouseRayDirectionWorld((float32)InputState->MouseEvent->PosX,
                                                           (float32)InputState->MouseEvent->PosY,
                                                           Window->Width,
                                                           Window->Height,
                                                           Camera->ProjectionMatrix,
                                                           camera::GetViewMatrix(Camera));

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
				g_HoveredObject = it->first;
				break;
			}
			else
				g_HoveredObject = 0;
		}
          
		glm::vec3 pIntersection = glm::vec3(0.0f);
		if (!RayPlaneIntersection(Camera->Position,
								  rayWorld, glm::vec3(0.0f),
								  glm::vec3(0.0f, 1.0f, 0.0f),
								  &pIntersection))
			pIntersection = glm::vec3(0.0f);

		// NOTE: RENDERING ======================================>
		renderer::ResetStats(Renderer);
		renderer::NewRenderingContext(Renderer);
		glm::mat4 viewMatrix = camera::GetViewMatrix(Camera);

		shader_t *ColorShader = shader::GetFromCache("color");
		shader::UseProgram(ColorShader);
		shader::SetUniform4fv(ColorShader, "view", viewMatrix);
		shader::SetUniform4fv(ColorShader, "model", glm::mat4(1.0f));

        // =================== D.E.B.U.G ===================
        
		// draw editor grid
		if (Editor->GridResolution != g_GridResolutionSlider)
		{
			PushGridSubData(Editor->MeshGrid, g_GridResolutionSlider, g_GridMaxResolution);
			Editor->GridResolution = g_GridResolutionSlider;
		}
		if (Editor->GridResolution > 0)
		{
			shader::SetUniform4f(ColorShader, "color", glm::vec4(0.360f, 0.360f, 0.360f, 1.0f));
			renderer::DrawLines(Renderer, Editor->MeshGrid, ColorShader);
		}

        // draw axis debug
        shader::SetUniform4f(ColorShader, "color", glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
        renderer::DrawLines(Renderer, MeshAxisDebug, ColorShader);

		// draw raycasting
		PushRaySubData(Editor->MeshRay, Camera->Position, rayWorld);
		shader::SetUniform4f(ColorShader, "color", glm::vec4(1.0f, 0.8f, 0.0f, 1.0));
		renderer::DrawLines(Renderer, Editor->MeshRay, ColorShader);

        // =================== M.O.D.E.L.S ===================

        // draw terrain
        if (!g_TerrainPrepared)
        {
            renderer::PrepareInstancedRendering(Renderer,
                                                terrainModel,
                                                terrainModelMatrices,
                                                g_TerrainSideLenght * g_TerrainSideLenght);
            g_TerrainPrepared = true;
        }
		shader_t *InstancedShader = shader::GetFromCache("instanced");
		shader::UseProgram(InstancedShader);
		shader::SetUniform4fv(InstancedShader, "view", viewMatrix);
		shader::SetUniform4fv(InstancedShader, "model", glm::mat4(1.0f));
        renderer::DrawModelInstanced(Renderer,
                                     terrainModel,
                                     InstancedShader,
                                     g_TerrainSideLenght * g_TerrainSideLenght);
        
                  
		// draw objs
		shader_t *DefaultShader = shader::GetFromCache("default");
		shader::UseProgram(DefaultShader);
		shader::SetUniform4fv(DefaultShader, "view", viewMatrix);
		shader::SetUniform4fv(DefaultShader, "model", glm::mat4(1.0f));
		for (auto it = SCENE->begin(); it != SCENE->end(); it++)
		{
			bool isSelected = false;
			if (g_HoveredObject == it->first || g_SelectedObject == it->first)
				isSelected = true;

			if (g_DragObject == it->first)
				it->second->Position = pIntersection;

			// Model
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, it->second->Position);
			model = glm::scale(model, glm::vec3(it->second->Scale));
			model = glm::rotate(model, glm::radians(it->second->Rotate),
								glm::vec3(0.0f, 1.0f, 0.0f));
			shader::SetUniform4fv(DefaultShader, "model", model);
			shader::SetUniform1ui(DefaultShader, "flip_color", isSelected);
			renderer::DrawModel(Renderer, it->second->Model, DefaultShader);

			// Picking sphere
			model = glm::mat4(1.0f);
			model = glm::translate(model, it->second->Position);
			shader::SetUniform4fv(DefaultShader, "model", model);
			shader::SetUniform1ui(DefaultShader, "flip_color", isSelected);
			renderer::DrawMesh(Renderer, it->second->PickingSphere, DefaultShader);
		}

        // =================== E.N.V.I.R.O.N.M.E.N.T ===================

		// Skybox
		if (Editor->Skybox != NULL && Editor->ShowSkybox)
		{
			shader_t *SkyboxShader = shader::GetFromCache("skybox");
			shader::UseProgram(SkyboxShader);
			shader::SetUniform4fv(SkyboxShader, "view", glm::mat4(glm::mat3(viewMatrix))); // remove translation from the view matrix
			renderer::DrawSkybox(Renderer, Editor->Skybox, SkyboxShader);
		}

        // =================== G.U.I ===================

		// draw GUI
		editorGUI::NewFrame();
		editorGUI::ShowWindowStatsOverlay(Window, Renderer);
		editorGUI::ShowEditorPanel(Window,
								   InputState,
								   Camera,
								   g_GridResolutionSlider,
								   g_GridMaxResolution,
								   &Editor->ShowSkybox,
								   SCENE,
								   &g_SelectedObject,
								   g_PickingSphereRadius,
								   g_ActiveWindow);

		editorGUI::Render();

		// TODO:  update memory pool
		//Renderer->MemoryArena->MaxUsed = 0;

		window::SwapBuffer(Window);
	}

    delete []terrainModelMatrices;
	for (auto it = SCENE->begin(); it != SCENE->end(); it++)
		object::Delete(it->second);
	delete SCENE;
    // TODO: implement complete delete method Editor
	mesh::Delete(Editor->MeshGrid);
    mesh::Delete(Editor->MeshAxisDebug);
	mesh::Delete(Editor->MeshRay);
	skybox::Delete(Editor->Skybox);
	delete Editor;

	editorGUI::Delete();
	shader::ClearCache();
	camera::Delete(Camera);
	renderer::Delete(Renderer);
	input::Delete(InputState);
	window::Delete(Window);
	return 0;
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

void PushRaySubData(mesh_t *Mesh, glm::vec3 origin, glm::vec3 direction)
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

bool RaySphereIntersection(glm::vec3 rayOriginWorld,
						   glm::vec3 rayDirectionWorld,
						   glm::vec3 sphereCenterWorld,
						   float32 sphereRadius,
						   float32 *intersectionDistance)
{
	// work out components of quadratic
	glm::vec3 distToSphere = rayOriginWorld - sphereCenterWorld;
	float32 b = dot(rayDirectionWorld, distToSphere);
	float32 c = dot(distToSphere, distToSphere) - sphereRadius * sphereRadius;
	float32 b_squared_minus_c = b * b - c;

	// check for "imaginary" answer. == ray completely misses sphere
	if (b_squared_minus_c < 0.0f)
	{
		return false;
	}

	// check for ray hitting twice (in and out of the sphere)
	if (b_squared_minus_c > 0.0f)
	{
		// get the 2 intersection distances along ray
		float32 t_a = -b + sqrt(b_squared_minus_c);
		float32 t_b = -b - sqrt(b_squared_minus_c);
		*intersectionDistance = t_b;

		// if behind viewer, throw one or both away
		if (t_a < 0.0)
		{
			if (t_b < 0.0)
			{
				return false;
			}
		}
		else if (t_b < 0.0)
		{
			*intersectionDistance = t_a;
		}

		return true;
	}

	// check for ray hitting once (skimming the surface)
	if (0.0f == b_squared_minus_c)
	{
		// if behind viewer, throw away
		float32 t = -b + sqrt(b_squared_minus_c);
		if (t < 0.0f)
		{
			return false;
		}
		*intersectionDistance = t;
		return true;
	}

	// note: could also check if ray origin is inside sphere radius
	return false;
}

// In case of plane (infint plane surface) it will be always intersec until the ray is parallel to the plane
bool RayPlaneIntersection(glm::vec3 rayOriginWorld,
						  glm::vec3 rayDirectionWorld, // Normalized !!!!
						  glm::vec3 planeCoord,
						  glm::vec3 planeNormal,
						  glm::vec3 *intersectionPoint)
{
	/*
      What does the d value mean ?
      For two vectors a and b a dot product actually returns the length of the orthogonal projection of one vector on the other times this other vector.
      But if a is normalized (length = 1), Dot(a, b) is then the length of the projection of b on a. In case of our plane, d gives us the directional distance all points of the plane in the normal direction to the origin (a is the normal). We can then get whether a point is on this plane by comparing the length of the projection on the normal (Dot product).
    */
	float32 d = dot(planeNormal, planeCoord);

	if (dot(planeNormal, rayDirectionWorld) == 0)
	{
		return false; // No intersection, the line is parallel to the plane
	}

	// Compute the X value for the directed line ray intersecting the plane
	float32 x = (d - dot(planeNormal, rayOriginWorld)) / dot(planeNormal, rayDirectionWorld);

	// output itersec point
	*intersectionPoint = rayOriginWorld + rayDirectionWorld * x;
	return true;
}

glm::mat4* GenerateTerrainModelMatrices(uint32 squareSideLenght)
{
    glm::mat4 *modelMatrices;
    modelMatrices = new glm::mat4[squareSideLenght * squareSideLenght];    
    glm::vec3 size = { 1.0f, 1.0f, 1.0f };
    float posX = 0.0f;

    uint32 index = 0;
    for (uint32 i = 0; i < squareSideLenght; i++)
    {
        float posZ = -size.z;
        for (uint32 y = 0; y < squareSideLenght; y++)
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, { posX, 0.0f, posZ });
            model = glm::scale(model, glm::vec3(1.0f));
            model = glm::rotate(model, glm::radians(0.0f),
                                glm::vec3(0.0f, 1.0f, 0.0f));
            modelMatrices[index] = model;

            posZ -= size.z;
            index++;
        }
        posX += size.x;
    }

    return modelMatrices;
}
