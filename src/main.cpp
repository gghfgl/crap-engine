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
   - scale model
   - list model
   - load model on the fly with imgui in a list WITH progress bar
   - improve grid rendering
*/

// -------------------------------

void PushRaySubData(mesh_t *Mesh, glm::vec3 origin, glm::vec3 direction);
void PushGridSubData(mesh_t *Mesh, uint32 resolution, uint32 maxResolution);
mesh_t* CreatePickingSphereMesh(float32 margin, float32 radius, uint32 stacks, uint32 slices);
bool RaySphereIntersection(glm::vec3 rayOriginWorld, glm::vec3 rayDirectionWorld, glm::vec3 sphereCenterWorld, float32 sphereRadius, float32 *intersectionDistance);
bool RayPlaneIntersection(glm::vec3 rayOriginWorld, glm::vec3 rayDirectionWorld, glm::vec3 planeCoord, glm::vec3 planeNormal, glm::vec3 *intersectionPoint);

// -------------------------------

struct editor_t
{
    bool Active;
    mesh_t *MeshGrid;
    mesh_t *MeshRay;
    uint32 GridResolution;
};

const uint32 g_Width = 1280;
const uint32 g_Height = 960;
static bool g_ActiveWindow = false;
glm::vec3 g_CameraStartPosition = glm::vec3(0.0f, 5.0f, 10.0f);
static const uint32 g_GridMaxResolution = 52;
static uint32 g_GridResolutionSlider = 10;
static uint32 g_HoveredObject = 0;
static uint32 g_SelectedObject = 0;
static uint32 g_DragObject = 0;
const float32 g_PickingSphereRadius = 0.1f;

static std::map<uint32, object_t*> SCENE_OBJECTS;

int main(int argc, char *argv[])
{
    window_t *Window = window::Construct(g_Width, g_Height, "crapEngine");
    input_t *InputState = input::Construct(Window->PlatformWindow);
    camera_t *Camera = camera::Construct((float32)g_Width, (float32)g_Height, g_CameraStartPosition);
    renderer_t *Renderer = renderer::Construct();

    shader::CompileAndCache("../shaders/default.glsl", "default", Camera->ProjectionMatrix);
    shader::CompileAndCache("../shaders/color.glsl", "color", Camera->ProjectionMatrix);

    // =================================================
    // Grid
    std::vector<vertex_t> vGrid(g_GridMaxResolution * 4, vertex_t());
    std::vector<uint32> uEmpty;
    std::vector<texture_t> tEmpty;
    mesh_t *MeshGrid = mesh::Construct(vGrid, uEmpty, tEmpty);

    // Ray
    std::vector<vertex_t> vRay(2, vertex_t());
    mesh_t *MeshRay = mesh::Construct(vRay, uEmpty, tEmpty);

    // Nanosuit with spheres pos
    const char* nanosuitFilepath = "../assets/nanosuit/nanosuit.obj";
    model_t *Nanosuit = model::LoadFromFile(nanosuitFilepath);
    object_t *TestObject = new object_t;
    TestObject->Label = "nanosuit";
    TestObject->Filepath = nanosuitFilepath;
    TestObject->Model = Nanosuit;
    TestObject->PickingSphere = CreatePickingSphereMesh(0.0f, g_PickingSphereRadius, 15, 15);
    TestObject->Position = glm::vec3(0.0f, 0.0f, 0.0f);

    // Object list
    SCENE_OBJECTS.insert({1, TestObject}); // TODO: ID->object
    // =================================================
    
    editor_t *Editor = new editor_t;
    Editor->Active = true;
    Editor->GridResolution = 0;
    Editor->MeshGrid = MeshGrid;
    Editor->MeshRay = MeshRay;
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
	
	if (InputState->MouseEvent->LeftButton)
	{
	    if (g_HoveredObject != 0)
		g_SelectedObject = g_HoveredObject;
	    else if (g_DragObject == 0)
		g_SelectedObject = 0;

	    if (g_SelectedObject != 0)
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

	for (auto it = SCENE_OBJECTS.begin(); it != SCENE_OBJECTS.end(); it++)
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

	// TODO: if editor obj select active
	shader_t *ColorShader = shader::GetFromCache("color");
	shader::UseProgram(ColorShader);
	shader::SetUniform4fv(ColorShader, "view", viewMatrix);    
	shader::SetUniform4fv(ColorShader, "model", glm::mat4(1.0f));

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

	// Test ray  drawing
	PushRaySubData(Editor->MeshRay, Camera->Position, rayWorld);
	shader::SetUniform4f(ColorShader, "color", glm::vec4(1.0f, 0.0f, 0.0f, 1.0));
	renderer::DrawLines(Renderer, Editor->MeshRay, ColorShader);

	shader_t *DefaultShader = shader::GetFromCache("default");
	shader::UseProgram(DefaultShader);
	shader::SetUniform4fv(DefaultShader, "view", viewMatrix);    
	shader::SetUniform4fv(DefaultShader, "model", glm::mat4(1.0f));
	for (auto it = SCENE_OBJECTS.begin(); it != SCENE_OBJECTS.end(); it++)
	{
	    bool isSelected = false;
	    if (g_HoveredObject == it->first || g_SelectedObject == it->first)
		isSelected = true;

	    if (g_DragObject == it->first)
		it->second->Position = pIntersection;

	    // Model
	    glm::mat4 model = glm::mat4(1.0f);
	    model = glm::translate(model, it->second->Position);
	    model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
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

	editorGUI::NewFrame();
	editorGUI::ShowWindowStatsOverlay(Window, Renderer);
	editorGUI::ShowEditorPanel(Window,
				   InputState,
				   g_GridResolutionSlider,
				   g_GridMaxResolution,
				   SCENE_OBJECTS,
				   &g_SelectedObject,
				   g_ActiveWindow);
	editorGUI::Render();
	
	// TODO:  update memory pool
	//Renderer->MemoryArena->MaxUsed = 0;
	
	window::SwapBuffer(Window);
    }

    for (auto it = SCENE_OBJECTS.begin(); it != SCENE_OBJECTS.end(); it++)
	object::Delete(it->second);
    mesh::Delete(Editor->MeshGrid);
    mesh::Delete(Editor->MeshRay);
    delete Editor;

    editorGUI::Delete();
    shader::ClearCache();
    camera::Delete(Camera);
    renderer::Delete(Renderer);
    input::Delete(InputState);
    window::Delete(Window);
    return 0;
}

void PushGridSubData(mesh_t *Mesh, uint32 resolution, uint32 maxResolution)
{
    if (resolution <= maxResolution)
    {
	uint32 vCount = resolution * 4 + 4; // 44
	float32 b = (float32)resolution / 2.0f + 1.0f; // 6
	float32 a = -b; // -6
	float32 xPos = -((float32)resolution / 2.0f); // -5
	float32 zPos = xPos; // -5

	Mesh->Vertices.clear();
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

	    Mesh->Vertices.push_back(v);
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

mesh_t* CreatePickingSphereMesh(float32 margin, float32 radius, uint32 stacks, uint32 slices)
{
    uint32 nbVerticesPerSphere = 0;
    std::vector<vertex_t> vertices;
    std::vector<uint32> indices;

    for (uint32 i = 0; i <= stacks; i++)
    {
	GLfloat V   = i / (float) stacks;
	GLfloat phi = V * glm::pi <float> ();
        
	for (uint32 j = 0; j <= slices; ++j)
	{
	    GLfloat U = j / (float) slices;
	    GLfloat theta = U * (glm::pi <float> () * 2);
            
	    // Calc The Vertex Positions
	    GLfloat x = cosf (theta) * sinf (phi);
	    GLfloat y = cosf (phi);
	    GLfloat z = sinf (theta) * sinf (phi);
	    vertex_t Vertex;
	    Vertex.Position = glm::vec3(x * radius + margin, y * radius, z * radius);
	    vertices.push_back(Vertex);
	    nbVerticesPerSphere += 1; // nb vertices per sphere reference
	}
    }

    for (uint32 i = 0; i < slices * stacks + slices; ++i)
    {        
	indices.push_back (i);
	indices.push_back (i + slices + 1);
	indices.push_back (i + slices);
        
	indices.push_back (i + slices + 1);
	indices.push_back (i);
	indices.push_back (i + 1);
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

bool RaySphereIntersection(glm::vec3 rayOriginWorld,
			   glm::vec3 rayDirectionWorld,
			   glm::vec3 sphereCenterWorld,
			   float32 sphereRadius,
			   float32 *intersectionDistance)
{
    // work out components of quadratic
    glm::vec3 distToSphere = rayOriginWorld - sphereCenterWorld;
    float32 b = dot( rayDirectionWorld, distToSphere );
    float32 c = dot( distToSphere, distToSphere ) - sphereRadius * sphereRadius;
    float32 b_squared_minus_c = b * b - c;

    // check for "imaginary" answer. == ray completely misses sphere
    if ( b_squared_minus_c < 0.0f ) { return false; }

    // check for ray hitting twice (in and out of the sphere)
    if ( b_squared_minus_c > 0.0f ) {
	// get the 2 intersection distances along ray
	float32 t_a = -b + sqrt( b_squared_minus_c );
	float32 t_b = -b - sqrt( b_squared_minus_c );
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
	float32 t = -b + sqrt( b_squared_minus_c );
	if ( t < 0.0f ) { return false; }
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
