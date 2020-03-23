#include "engine.h"
#include "resource_manager.h"

engine* engine_construct() {
    engine* Result = new engine();
    Result->GlobalState = ENGINE_ACTIVE;
    Result->Width = 0;
    Result->Height = 0;

    return Result;
}

void delete_engine(engine *Engine) {
    delete_input_state(Engine->InputState);
    delete_camera(Engine->Camera);
    delete_renderer(Engine->Renderer);
    delete Engine;
}

void init_engine_data(engine *Engine, GLFWwindow* Window, unsigned int width, unsigned int height) {
    // TODO: improve init of camera and input?
    camera *Camera = camera_construct(glm::vec3(0.0f, 0.0f, 0.0f));
    input_state *InputState = input_state_construct(Window, width, height);

    Engine->InputState = InputState;
    Engine->Camera = Camera;
    Engine->Width = width;
    Engine->Height = height;

    // TODO: should load shaders from config or parameters?
    ResourceManager::LoadShader("../shaders/camera.vs", "../shaders/camera.fs", nullptr, "camera");

    // Configure shaders
    // be sure to activate shader when setting uniforms/drawing objects
    Shader cameraShader = ResourceManager::GetShader("camera")->Use();

    // be sure to activate shader when setting uniforms/drawing objects
    cameraShader.SetVector3f("lightColor",  1.0f, 1.0f, 1.0f);

    // view/projection transformations
    glm::mat4 projection = glm::perspective(glm::radians(Engine->Camera->Zoom), (float)Engine->Width / (float)Engine->Height, 0.1f, 100.0f);
    cameraShader.SetMatrix4("projection", projection);
    
    // Set render-specific controls
    Engine->Renderer = renderer_construct(ResourceManager::GetShader("camera"));;
}

void update_engine(engine *Engine, float deltaTime) {
    // here ...
}

void process_input_event(engine *Engine, float deltaTime) {
    if (Engine->GlobalState == ENGINE_ACTIVE) {
	if (Engine->InputState->Keyboard[GLFW_KEY_W]) {
	    process_camera_keyboard(Engine->Camera, FORWARD, deltaTime);
	}

	if (Engine->InputState->Keyboard[GLFW_KEY_S]) {
	    process_camera_keyboard(Engine->Camera, BACKWARD, deltaTime);
	}

	if (Engine->InputState->Keyboard[GLFW_KEY_A]) {
	    process_camera_keyboard(Engine->Camera, LEFT, deltaTime);
	}

	if (Engine->InputState->Keyboard[GLFW_KEY_D]) {
	    process_camera_keyboard(Engine->Camera, RIGHT, deltaTime);
	}
	
	if (Engine->InputState->MouseLeftButton) {
	    update_mouse_offset(Engine->InputState);
	    process_camera_mouse_movement(Engine->Camera, Engine->InputState->MouseOffsetX, Engine->InputState->MouseOffsetY);
	}
    }
}

void global_render(engine *Engine) {
    if(Engine->GlobalState == ENGINE_ACTIVE) {
        renderer_draw(Engine->Renderer, Engine->Camera, glm::vec3( 0.0f,  0.0f,  0.0f), glm::vec3( 1.0f,  0.0f,  0.0f));
    }
}
