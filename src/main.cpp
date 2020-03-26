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

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;


    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

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
	for (float i = 0.0f; i < 10.0f; i += 1.0f) {
	    add_to_cube_buffer(
		Engine->Renderer,
		{ 0.0f, 0.0f, -pos },
		{ 2.0f, 2.0f, 2.0f },
		{ r, g, 0.0f, 1.0f });

	    r += 0.05f;
	    g += 0.05f;
	    pos += 2.0f;
	}

	close_cube_batch(Engine->Renderer);
	flush_cube_batch(Engine->Renderer);

	if (Engine->OverlayState == OVERLAY_DEBUG) {
	    ImGui_ImplOpenGL3_NewFrame();
	    ImGui_ImplGlfw_NewFrame();
	    ImGui::NewFrame();

	    {
		const float DISTANCE = 10.0f;
		static int corner = 0;
		ImGuiIO& io = ImGui::GetIO();
		if (corner != -1)
		{
		    ImVec2 window_pos = ImVec2((corner & 1) ? io.DisplaySize.x - DISTANCE : DISTANCE, (corner & 2) ? io.DisplaySize.y - DISTANCE : DISTANCE);
		    ImVec2 window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
		    ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
		}

		ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
		if (ImGui::Begin("Debug overlay", NULL, (corner != -1 ? ImGuiWindowFlags_NoMove : 0) | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav))
		{
		    ImGui::Text("Debug overlay");
		    ImGui::Separator();
		    ImGui::Text((const char*)glGetString(GL_VENDOR));
		    ImGui::Text((const char*)glGetString(GL_RENDERER));
		    ImGui::Text((const char*)glGetString(GL_VERSION));
		    ImGui::Separator();
		    if (ImGui::IsMousePosValid())
			ImGui::Text("Mouse Position: (%.1f,%.1f)", io.MousePos.x, io.MousePos.y);
		    else
			ImGui::Text("Mouse Position: <invalid>");
		    ImGui::Text("CubeCount = %d", Engine->Renderer->Stats.CubeCount);
		    ImGui::Text("DrawCount = %d", Engine->Renderer->Stats.DrawCount);
		    ImGui::End();
		}
	    }	

	    ImGui::Render();
	    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

	stop_rendering(Engine);
	// DEBUG ==============================================================================	
    }

    // Delete
    delete_imgui();
    delete_engine(Engine);

    return 0;
}
