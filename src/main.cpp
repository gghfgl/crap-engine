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
   - malloc
   - finish draw grid mesh with dyn parameter and GUI
   - implement mesh with load model model
   - scale and move model
*/

bool g_EditorActive = true;
const uint32 g_Width = 1280;
const uint32 g_Height = 960;
static bool g_ActiveWindow = false;
glm::vec3 g_CameraStartPosition = glm::vec3(0.0f, 5.0f, 10.0f);

mesh_t* LoadDebugGrid();

int main(int argc, char *argv[])
{
    // Init all systems
    window_t *Window = window::Construct(g_Width, g_Height, "crapEngine");
    input_t *InputState = input::Construct(Window->PlatformWindow);
    camera_t *Camera = camera::Construct((float32)g_Width, (float32)g_Height, g_CameraStartPosition);
    renderer_t *Renderer = renderer::Construct();
    shader::CompileAndCache("../shaders/default.vs", "../shaders/default.fs", nullptr,
			  "default", Camera->ProjectionMatrix);
    shader::CompileAndCache("../shaders/default.vs", "../shaders/stencil.fs", nullptr,
    			  "stencil", Camera->ProjectionMatrix);
    EditorGUI::Init(Window);

    // TODO: 
    mesh_t *MeshGrid = LoadDebugGrid();

    while (g_EditorActive)
    {
	window::UpdateTime(Window->Time);

	// NOTE: INPUTS ======================================>
	input::PollEvents();
	if (InputState->KeyboardEvent->IsPressed[CRAP_KEY_ESCAPE])
	    g_EditorActive = false;
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

	// NOTE: RENDERING ======================================>
	renderer::NewRenderingContext(Renderer,
				      shader::GetFromCache("default"),
				      camera::GetViewMatrix(Camera));

	renderer::DrawLines(Renderer, MeshGrid);
	
	/* TODO: 
	   -load assets (load at start) model-mesh / shaders / textures / etc ...
	   - generate vertex data CPU / GPU ?...
	   - bind program shader + vertex data + uinifor
	*/

	EditorGUI::NewFrame();
	EditorGUI::ShowWindowStatsOverlay(Window);
	EditorGUI::ShowSettingsPanel(Window, g_ActiveWindow);
	EditorGUI::Render();
	
	// TODO:  update memory pool
	//Renderer->MemoryArena->MaxUsed = 0;
	
	window::SwapBuffer(Window);
    }

    EditorGUI::Delete();
    shader::ClearCache();
    camera::Delete(Camera);
    renderer::Delete(Renderer);
    input::Delete(InputState);
    window::Delete(Window);
    return 0;
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

// TODO: add dynamic size
mesh_t* LoadDebugGrid()
{    
    float debug_grid[] =
	{
	    -6.0f, 0.0f, -5.0f, 0.0f, 0.0f, 1.0f, 1.0f,
	    6.0f, 0.0f, -5.0f, 1.0f, 0.0f, 0.0f, 1.0f,

	    -6.0f, 0.0f, -4.0f, 0.0f, 0.0f, 1.0f, 1.0f,
	    6.0f, 0.0f, -4.0f, 1.0f, 0.0f, 0.0f, 1.0f,

	    -6.0f, 0.0f, -3.0f, 0.0f, 0.0f, 1.0f, 1.0f,
	    6.0f, 0.0f, -3.0f, 1.0f, 0.0f, 0.0f, 1.0f,

	    -6.0f, 0.0f, -2.0f, 0.0f, 0.0f, 1.0f, 1.0f,
	    6.0f, 0.0f, -2.0f, 1.0f, 0.0f, 0.0f, 1.0f,

	    -6.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
	    6.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,

	    -6.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,
	    6.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,

	    -6.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
	    6.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,

	    -6.0f, 0.0f, 2.0f, 0.0f, 0.0f, 1.0f, 1.0f,
	    6.0f, 0.0f, 2.0f, 1.0f, 0.0f, 0.0f, 1.0f,

	    -6.0f, 0.0f, 3.0f, 0.0f, 0.0f, 1.0f, 1.0f,
	    6.0f, 0.0f, 3.0f, 1.0f, 0.0f, 0.0f, 1.0f,

	    -6.0f, 0.0f, 4.0f, 0.0f, 0.0f, 1.0f, 1.0f,
	    6.0f, 0.0f, 4.0f, 1.0f, 0.0f, 0.0f, 1.0f,

	    -6.0f, 0.0f, 5.0f, 0.0f, 0.0f, 1.0f, 1.0f,
	    6.0f, 0.0f, 5.0f, 1.0f, 0.0f, 0.0f, 1.0f,

	    // =======================================
	    
	    -5.0f, 0.0f, 6.0f, 1.0f, 0.0f, 0.0f, 1.0f,
	    -5.0f, 0.0f, -6.0f, 0.0f, 0.0f, 1.0f, 1.0f,
	    
	    -4.0f, 0.0f, 6.0f, 1.0f, 0.0f, 0.0f, 1.0f,
	    -4.0f, 0.0f, -6.0f, 0.0f, 0.0f, 1.0f, 1.0f,
	    
	    -3.0f, 0.0f, 6.0f, 1.0f, 0.0f, 0.0f, 1.0f,
	    -3.0f, 0.0f, -6.0f, 0.0f, 0.0f, 1.0f, 1.0f,
	    
	    -2.0f, 0.0f, 6.0f, 1.0f, 0.0f, 0.0f, 1.0f,
	    -2.0f, 0.0f, -6.0f, 0.0f, 0.0f, 1.0f, 1.0f,
	    
	    -1.0f, 0.0f, 6.0f, 1.0f, 0.0f, 0.0f, 1.0f,
	    -1.0f, 0.0f, -6.0f, 0.0f, 0.0f, 1.0f, 1.0f,
	    
	    0.0f, 0.0f, 6.0f, 1.0f, 0.0f, 0.0f, 1.0f,
	    0.0f, 0.0f, -6.0f, 0.0f, 0.0f, 1.0f, 1.0f,
	    
	    1.0f, 0.0f, 6.0f, 1.0f, 0.0f, 0.0f, 1.0f,
	    1.0f, 0.0f, -6.0f, 0.0f, 0.0f, 1.0f, 1.0f,
	    
	    2.0f, 0.0f, 6.0f, 1.0f, 0.0f, 0.0f, 1.0f,
	    2.0f, 0.0f, -6.0f, 0.0f, 0.0f, 1.0f, 1.0f,
	    
	    3.0f, 0.0f, 6.0f, 1.0f, 0.0f, 0.0f, 1.0f,
	    3.0f, 0.0f, -6.0f, 0.0f, 0.0f, 1.0f, 1.0f,
	    
	    4.0f, 0.0f, 6.0f, 1.0f, 0.0f, 0.0f, 1.0f,
	    4.0f, 0.0f, -6.0f, 0.0f, 0.0f, 1.0f, 1.0f,
	    
	    5.0f, 0.0f, 6.0f, 1.0f, 0.0f, 0.0f, 1.0f,
	    5.0f, 0.0f, -6.0f, 0.0f, 0.0f, 1.0f, 1.0f
	};

    mesh_t *Mesh = new mesh_t;
    Mesh->VertexCount = 44;

    glGenVertexArrays(1, &Mesh->VAO);
    glBindVertexArray(Mesh->VAO);

    glGenBuffers(1, &Mesh->VBO);
    glBindBuffer(GL_ARRAY_BUFFER, Mesh->VBO);
    // TODO: dynamic
    glBufferData(GL_ARRAY_BUFFER, sizeof(debug_grid), debug_grid, GL_STATIC_DRAW);

    // Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float32), (void*)0);

    // Color
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float32), (void*)(3 * sizeof(float32)));

    return Mesh;
}
