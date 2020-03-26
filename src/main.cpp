#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "plateform.h"

// TODO:
// * implement frame rate counter ms
// * implement game entity and level?
// * in fuction time profiler (handemade hero fast thread id retrieval)
// * work on z-buffer to avoid z-fighting colors/ textures
// * add texture and white default texture to batch rendering cube.
// * generate terrain?
// * stencil test outline object
// * compile and read shader for presetting uniform? the cherno
// * implement logger? check the cherno
// * add batch model renderer?
// * memory profiler
// * light system PBR?



int main(int argc, char *argv[]) {
    // Initialize engine
    engine *Engine = engine_construct();
    init_engine_data(Engine, 1280, 960, NO_DEBUG);
    init_imgui(Engine->Window);

    prepare_debug_rendering(Engine->Renderer);
    prepare_cube_batch_rendering(Engine->Renderer);

    // needed for enforcing 60fps
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    while (Engine->GlobalState == ENGINE_ACTIVE) {
	// DEBUG ==============================================================================
	engine_update(Engine, deltaTime);

	// DeltaTime TODO build a function to auto retrieve this?
	float currentFrame = (float)glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;

	if (Engine->InputState->Keyboard[GLFW_KEY_ESCAPE]) {
	    Engine->GlobalState = ENGINE_TERMINATE;
	}

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

	// Render
	start_rendering(Engine);
	
	draw_debug(Engine->Renderer);
	
	reset_renderer_stats(Engine->Renderer);
	start_new_cube_batch(Engine->Renderer);

	float r = 0.12f;
	float g = 0.25f;
	float pos = 2.0f;
	for (float i = 0.0f; i < 1000.0f; i += 1.0f) {
	    add_to_cube_buffer(
		Engine->Renderer,
		{ 0.0f, 0.0f, -pos },
		{ 2.0f, 2.0f, 2.0f },
		{ r, g, 0.0f, 1.0f });

	    r += 0.05f;
	    g += 0.05f;
	    pos += 2.0f;
	}

	float r2 = 0.12f;
	float b2 = 0.25f;
	float pos2 = 2.0f;
	for (float i = 0.0f; i < 100.0f; i += 1.0f) {
	    add_to_cube_buffer(
		Engine->Renderer,
		{ pos2, 0.0f, 0.0f },
		{ 2.0f, 2.0f, 2.0f },
		{ r2, 0.0f, b2, 1.0f });

	    r2 += 0.05f;
	    b2 += 0.05f;
	    pos2 += 2.0f;
	}

	
	close_cube_batch(Engine->Renderer);
	flush_cube_batch(Engine->Renderer);

	if (Engine->OverlayState == OVERLAY_DEBUG) {
	    display_debug_overlay(Engine, deltaTime);
	}

	stop_rendering(Engine);
	// DEBUG ==============================================================================
    }

    // Delete
    delete_engine(Engine);

    return 0;
}
