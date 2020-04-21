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

/* TODO:
   - assimp be carefull of texture path (same dir as the model)
   - 3d picking + model list + select model from list + move/scale
   - load model on the fly with imgui in a list WITH progress bar
   - improve grid rendering
*/

void PushGridSubData(mesh_t *MeshGrid, uint32 resolution, uint32 maxResolution);
mesh_t* CreatePickingSpheresMesh(float radius, int stacks, int slices);
// -------------------------------
struct editor_t
{
    bool Active;
    mesh_t *MeshGrid;
    uint32 GridResolution;
};

struct object_t
{
    model_t *Model;
    mesh_t *PickingSpheres;
};

const uint32 g_Width = 1280;
const uint32 g_Height = 960;
static bool g_ActiveWindow = false;
glm::vec3 g_CameraStartPosition = glm::vec3(0.0f, 5.0f, 10.0f);
static const uint32 g_GridMaxResolution = 52;
static uint32 g_GridResolutionSlider = 10;

static std::map<uint32, object_t*> SCENE_OBJECTS;

int main(int argc, char *argv[])
{
    window_t *Window = window::Construct(g_Width, g_Height, "crapEngine");
    input_t *InputState = input::Construct(Window->PlatformWindow);
    camera_t *Camera = camera::Construct((float32)g_Width, (float32)g_Height, g_CameraStartPosition);
    renderer_t *Renderer = renderer::Construct();

    shader::CompileAndCache("../shaders/default.glsl", "default", Camera->ProjectionMatrix);
    shader::CompileAndCache("../shaders/grid.glsl", "grid", Camera->ProjectionMatrix);

    // =================================================
    // Grid
    std::vector<vertex_t> vGrid(g_GridMaxResolution * 4, vertex_t());
    std::vector<uint32> uEmpty;
    std::vector<texture_t> tEmpty;
    mesh_t *MeshGrid = mesh::Construct(vGrid, uEmpty, tEmpty);

    // Nanosuit with spheres pos
    model_t *Nanosuit = model::LoadFromFile("../assets/nanosuit/nanosuit.obj");
    object_t *FirstObject = new object_t;
    FirstObject->Model = Nanosuit;
    FirstObject->PickingSpheres = CreatePickingSpheresMesh(0.5f, 20, 20);
    SCENE_OBJECTS.insert({1, FirstObject}); // TODO: ID->object

    // =================================================

    
    editor_t *Editor = new editor_t;
    Editor->Active = true;
    Editor->GridResolution = 0;
    Editor->MeshGrid = MeshGrid;
    editorGUI::Init(Window);


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

	if (InputState->MouseEvent->LeftButton && !g_ActiveWindow)
	{
	    input::UpdateMouseOffsets(InputState->MouseEvent);
	    camera::ProcessMovementAngles(Camera,
				       InputState->MouseEvent->OffsetX,
				       InputState->MouseEvent->OffsetY);
	}

	// NOTE: SIMULATE  ======================================>

	// NOTE: RENDERING ======================================>
	renderer::ResetStats(Renderer);
	renderer::NewRenderingContext(Renderer);

	if (Editor->GridResolution != g_GridResolutionSlider)
	{
	    PushGridSubData(Editor->MeshGrid, g_GridResolutionSlider, g_GridMaxResolution);
	    Editor->GridResolution = g_GridResolutionSlider;
	}
	if (Editor->GridResolution > 0)
	    renderer::DrawLines(Renderer,
				Editor->MeshGrid,
				shader::GetFromCache("grid"),
				camera::GetViewMatrix(Camera));

	for (auto it = SCENE_OBJECTS.begin(); it != SCENE_OBJECTS.end(); it++)
	{
	    // TODO: if object positions changed then PushObjectSubData (model + sphere + axis.ord)
	    
	    glm::mat4 model = glm::mat4(1.0f);
	    //model = glm::translate(model, glm::vec3(0.0f, -1.75f, 0.0f));
	    model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
	    // renderer::DrawModel(Renderer,
	    // 			it->second->Model,
	    // 			shader::GetFromCache("default"),
	    // 			camera::GetViewMatrix(Camera),
	    // 			model);

	    // TODO: if object selected then render speheres and axis.ord
	    renderer::DrawMesh(Renderer,
	    		       it->second->PickingSpheres,
	    		       shader::GetFromCache("grid"),
	    		       camera::GetViewMatrix(Camera),
	    		       glm::mat4(1.0f));
	}

	editorGUI::NewFrame();
	editorGUI::ShowWindowStatsOverlay(Window);
	editorGUI::ShowSettingsPanel(Window,
				     g_GridResolutionSlider,
				     g_GridMaxResolution,
				     g_ActiveWindow);
	editorGUI::Render();
	
	// TODO:  update memory pool
	//Renderer->MemoryArena->MaxUsed = 0;
	
	window::SwapBuffer(Window);
    }

    for (auto it = SCENE_OBJECTS.begin(); it != SCENE_OBJECTS.end(); it++)
    {
	model::Delete(it->second->Model);
	mesh::Delete(it->second->PickingSpheres);
    }
    mesh::Delete(Editor->MeshGrid);
    delete Editor;

    editorGUI::Delete();
    shader::ClearCache();
    camera::Delete(Camera);
    renderer::Delete(Renderer);
    input::Delete(InputState);
    window::Delete(Window);
    return 0;
}

void PushGridSubData(mesh_t *MeshGrid, uint32 resolution, uint32 maxResolution)
{
    if (resolution <= maxResolution)
    {
	uint32 vCount = resolution * 4 + 4; // 44
	float32 b = (float32)resolution / 2.0f + 1.0f; // 6
	float32 a = -b; // -6
	float32 xPos = -((float32)resolution / 2.0f); // -5
	float32 zPos = xPos; // -5

	MeshGrid->Vertices.clear();
	uint32 i = 0;
	while (i < vCount / 2)
	{
	    vertex_t v;
	    if (i%2 == 0)
	    {
		v.Position = glm::vec3(a, 0.0f, zPos);
	    }
	    else
	    {
		v.Position = glm::vec3(b, 0.0f, zPos);
		zPos += 1.0f;
	    }

	    MeshGrid->Vertices.push_back(v);
	    i++;
	}

	while (i < vCount)
	{
	    vertex_t v;
	    if (i%2 == 0)
	    {
		v.Position = glm::vec3(xPos, 0.0f, a);
	    }
	    else
	    {
		v.Position = glm::vec3(xPos, 0.0f, b);
		xPos += 1.0f;
	    }

	    MeshGrid->Vertices.push_back(v);
	    i++;
	}
    
	glBindBuffer(GL_ARRAY_BUFFER, MeshGrid->VBO);
	glBufferSubData(GL_ARRAY_BUFFER,
			0,
			MeshGrid->Vertices.size() * sizeof(vertex_t),
			&MeshGrid->Vertices[0]);
    }
}

mesh_t* CreatePickingSpheresMesh(float radius, int stacks, int slices)
{
    float32 margin = 4.0f;
    uint32 nbVerticesPerSphere = 0;
    std::vector<vertex_t> vertices;
    std::vector<uint32> indices;

    for (uint32 m = 0; m < 3; m++)
    {
	for (int i = 0; i <= stacks; i++)
	{
	    GLfloat V   = i / (float) stacks;
	    GLfloat phi = V * glm::pi <float> ();
        
	    for (int j = 0; j <= slices; ++j)
	    {
		GLfloat U = j / (float) slices;
		GLfloat theta = U * (glm::pi <float> () * 2);
            
		// Calc The Vertex Positions
		GLfloat x = cosf (theta) * sinf (phi);
		GLfloat y = cosf (phi);
		GLfloat z = sinf (theta) * sinf (phi);
		vertex_t Vertex;
		if (m == 0)
		{
		    Vertex.Position = glm::vec3(x * radius + margin, y * radius, z * radius);
		    nbVerticesPerSphere += 1; // nb vertices per sphere reference
		}
		else if (m == 1)
		    Vertex.Position = glm::vec3(x * radius, y * radius + margin, z * radius);
		else if (m == 2)
		    Vertex.Position = glm::vec3(x * radius, y * radius, z * radius + margin);

		vertices.push_back(Vertex);
	    }
	}
    }

    uint32 offset = 0;
    for (uint32 a = 0; a < vertices.size(); a++)
    {
	for (int i = 0; i < slices * stacks + slices; ++i)
	{        
	    indices.push_back (i + offset);
	    indices.push_back (i + slices + 1 + offset);
	    indices.push_back (i + slices + offset);
        
	    indices.push_back (i + slices + 1 + offset);
	    indices.push_back (i + offset);
	    indices.push_back (i + 1 + offset);
	}

	offset += nbVerticesPerSphere;
    }

    std::vector<texture_t> tEmpty;
    mesh_t *Mesh = mesh::Construct(vertices, indices, tEmpty);

    return Mesh;
}

// NOTE: EDITOR stuff
// // TODO: use for moving / scaling model ?
// void LoadDebugAxis(uint32 *VAO, uint32 *vCount)
// {    
//     float debug_axis[] =
// 	{
// 	    0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
// 	    3.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,

// 	    0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
// 	    0.0f, 3.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
	
// 	    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,
// 	    0.0f, 0.0f, -3.0f, 0.0f, 0.0f, 1.0f, 1.0f
// 	};

//     *vCount = 6;

//     glGenVertexArrays(1, &VAO);
//     glBindVertexArray(VAO);

//     uint32 VBO;
//     glGenBuffers(1, &VBO);
//     glBindBuffer(GL_ARRAY_BUFFER, VBO);
//     glBufferData(GL_ARRAY_BUFFER, sizeof(debug_axis), debug_axis, GL_STATIC_DRAW);

//     // Position
//     glEnableVertexAttribArray(0);
//     glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float32), (void*)0);

//     // Color
//     glEnableVertexAttribArray(1);
//     glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float32), (void*)(3 * sizeof(float32)));
// }
