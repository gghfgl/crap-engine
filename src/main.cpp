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
   - refact reorganize code (renderer / mesh / model)
   - load model on the fly with imgui in a list
   - scale and move model
   - improve grid rendering
*/

void GenerateGridData(mesh_t *MeshGrid, uint32 resolution);
// -------------------------------
struct editor_t
{
    bool Active;
    mesh_t *MeshGrid;
    uint32 GridResolution;
};

const uint32 g_Width = 1280;
const uint32 g_Height = 960;
static bool g_ActiveWindow = false;
glm::vec3 g_CameraStartPosition = glm::vec3(0.0f, 5.0f, 10.0f);
static uint32 g_GridResolutionSlider = 10;

int main(int argc, char *argv[])
{
    window_t *Window = window::Construct(g_Width, g_Height, "crapEngine");
    input_t *InputState = input::Construct(Window->PlatformWindow);
    camera_t *Camera = camera::Construct((float32)g_Width, (float32)g_Height, g_CameraStartPosition);
    renderer_t *Renderer = renderer::Construct();
    shader::CompileAndCache("../shaders/default.vs", "../shaders/default.fs", nullptr,
			  "default", Camera->ProjectionMatrix);
    shader::CompileAndCache("../shaders/default.vs", "../shaders/stencil.fs", nullptr,
    			  "stencil", Camera->ProjectionMatrix);

    // =================================================
    mesh_t *MeshGrid = new mesh_t;
    MeshGrid->Data = new vertex_t[1000]; // TODO: ptr from memory pool ???

    glGenVertexArrays(1, &MeshGrid->VAO);
    glBindVertexArray(MeshGrid->VAO);

    glGenBuffers(1, &MeshGrid->VBO);
    glBindBuffer(GL_ARRAY_BUFFER, MeshGrid->VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_t) * 1000, nullptr, GL_DYNAMIC_DRAW);

    // Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
    			  sizeof(vertex_t),
    			  (const void*)offsetof(vertex_t, Position));

    // Color
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE,
    			  sizeof(vertex_t),
    			  (const void*)offsetof(vertex_t, Color));

    glBindVertexArray(0);
    // =================================================
    model_t *TestModel = model::LoadFromFile("../assets/nanosuit/nanosuit.obj");

    
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
	    GenerateGridData(Editor->MeshGrid, g_GridResolutionSlider);
	    Editor->GridResolution = g_GridResolutionSlider;
	}

	if (Editor->GridResolution > 0)
	    renderer::DrawLines(Renderer,
				Editor->MeshGrid,
				shader::GetFromCache("default"),
				camera::GetViewMatrix(Camera));

        glm::mat4 model = glm::mat4(1.0f);
        //model = glm::translate(model, glm::vec3(0.0f, -1.75f, 0.0f));
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
	renderer::DrawModel(Renderer,
			    TestModel,
			    shader::GetFromCache("default"),
			    camera::GetViewMatrix(Camera),
			    model);

	editorGUI::NewFrame();
	editorGUI::ShowWindowStatsOverlay(Window);
	editorGUI::ShowSettingsPanel(Window, g_GridResolutionSlider, g_ActiveWindow);
	editorGUI::Render();
	
	// TODO:  update memory pool
	//Renderer->MemoryArena->MaxUsed = 0;
	
	window::SwapBuffer(Window);
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

void GenerateGridData(mesh_t *MeshGrid, uint32 resolution)
{
    uint32 vCount = resolution * 4 + 4; // 44
    float32 b = (float32)resolution / 2.0f + 1.0f; // 6
    float32 a = -b; // -6
    float32 xPos = -((float32)resolution / 2.0f); // -5
    float32 zPos = xPos; // -5
    glm::vec4 color = glm::vec4(0.360f, 0.360f, 0.360f, 1.0f);

    // TODO: carefull vCount > maxVertex from memory pool
    MeshGrid->VertexCount = vCount;
    MeshGrid->DataPtr = MeshGrid->Data;

    uint32 i = 0;
    while (i < vCount / 2)
    {
	if (i%2 == 0)
	{
	    MeshGrid->DataPtr->Position = glm::vec3(a, 0.0f, zPos);
	    MeshGrid->DataPtr->Color = color;
	}
	else
	{
	    MeshGrid->DataPtr->Position = glm::vec3(b, 0.0f, zPos);
	    MeshGrid->DataPtr->Color = color;
	    zPos += 1.0f;
	}

	MeshGrid->DataPtr++;
	i++;
    }

    while (i < vCount)
    {
	if (i%2 == 0)
	{
	    MeshGrid->DataPtr->Position = glm::vec3(xPos, 0.0f, a);
	    MeshGrid->DataPtr->Color = color;
	}
	else
	{
	    MeshGrid->DataPtr->Position = glm::vec3(xPos, 0.0f, b);
	    MeshGrid->DataPtr->Color = color;
	    xPos += 1.0f;
	}

	MeshGrid->DataPtr++;
	i++;
    }
    
    GLsizeiptr size = (uint8*)MeshGrid->DataPtr - (uint8*)MeshGrid->Data;
    glBindBuffer(GL_ARRAY_BUFFER, MeshGrid->VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, size, MeshGrid->Data);
}
