#pragma once

static void window_settings_collapse_header(window_t *Window);
static void editor_grid_collapse_header(uint32 &resolution);

namespace editorGUI
{
    void Init(window_t* Window)
    {
	const char* glsl_version = "#version 450";
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	//ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(Window->PlatformWindow, true);
	ImGui_ImplOpenGL3_Init(glsl_version);
    }

    void Delete()
    {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
    }

    void NewFrame()
    {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
    }

    void Render()
    {
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    // TODO: switch to string rendering
    void ShowWindowStatsOverlay(window_t *Window)
    {
	ImGui::SetNextWindowPos(ImVec2((float32)Window->Width - 210, 10));
	ImGui::SetNextWindowBgAlpha(0.35f);
	if (ImGui::Begin("window_stats", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav))
	{
	    ImGui::Text("Window Stats");
	    ImGui::Separator();
	    ImGui::Text(Window->APIinfo.GPUvendor);
	    ImGui::Text(Window->APIinfo.Version);
	    ImGui::Separator();
	    ImGui::Text("ms/f: %.3fms", Window->Time->MsPerFrame);
	    //ImGui::Text("fps: %d", Window->Time->FPS);
	    ImGui::Text("mcy/f: %d", Window->Time->MegaCyclePerFrame);
	    ImGui::End();
	}
    }

    void ShowSettingsPanel(window_t *Window, uint32 &gridResolution, bool &focus)
    {
	ImGui::SetNextWindowPos(ImVec2(10, 10));
	ImGui::SetNextWindowSize(ImVec2(410, (float32)Window->Height - 20));
	ImGui::Begin("settings", nullptr, ImGuiWindowFlags_NoResize);

        window_settings_collapse_header(Window);
	editor_grid_collapse_header(gridResolution);
    
	if (ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow))
	    focus = true;
	else
	    focus = false;
	ImGui::End();
    }
}

static void window_settings_collapse_header(window_t *Window)
{
    if (ImGui::CollapsingHeader("Window settings", ImGuiTreeNodeFlags_DefaultOpen))
    {
    	ImVec2 bSize(40, 20);
	ImGui::Text("SCREEN: %d x %d", Window->Width, Window->Height);
	ImGui::Separator();

	ImGui::PushID(1);
	if (ImGui::Button(Window->Vsync ? "on" : "off", bSize))
	    window::ToggleVsync(Window);
	ImGui::SameLine();
	ImGui::Text("VSYNC: ");
	ImGui::PopID();

	ImGui::PushID(2);
	if (ImGui::Button(Window->DebugMode ? "on" : "off", bSize))
	    window::ToggleDebugMode(Window);
	ImGui::SameLine();
	ImGui::Text("DEBUG: ");
	ImGui::PopID();
    }
}

static void editor_grid_collapse_header(uint32 &resolution)
{
    if (ImGui::CollapsingHeader("Grid", ImGuiTreeNodeFlags_DefaultOpen))
    {
	ImGui::SliderInt("res", &(int)resolution, 0, 50);
	ImGui::Separator();
    }
}

    /* // TODO: GUI */
    /* static void CameraSettingsCollapseHeader(camera *Camera) */
    /* { */
    /* 	if (ImGui::CollapsingHeader("Camera settings", ImGuiTreeNodeFlags_DefaultOpen)) */
    /* 	{ */
    /* 	    ImGui::Text("yaw: %.2f", Camera->Settings->Yaw); */
    /* 	    ImGui::Text("pitch: %.2f", Camera->Settings->Pitch); */
    /* 	    ImGui::Text("speed: %.2f", Camera->Settings->Speed); */
    /* 	    ImGui::Text("sensitivity: %.2f", Camera->Settings->Sensitivity); */
    /* 	    ImGui::Text("fov: %.2f", Camera->Settings->Fov); */
    /* 	    ImGui::Text("pos: %.2f, %.2f, %.2f", */
    /* 			Camera->Position.x, */
    /* 			Camera->Position.y, */
    /* 			Camera->Position.z); */

    /* 	    ImVec2 bSize(100, 20); */
    /* 	    ImGui::Button("Reset Default", bSize); */
    /* 	    ImGui::SameLine(); */
    /* 	    ImGui::Button("Reset Front", bSize); */
    /* 	    ImGui::SameLine(); */
    /* 	    ImGui::Button("Reset Up", bSize); */
    /* 	    ImGui::Separator(); */
    /* 	} */
    /* } */

/* // TODO: move to GUI file */
/*     static void InputStateSettingsCollapseHeader(input_t *InputState) */
/*     { */
/* 	if (ImGui::CollapsingHeader("Input settings", ImGuiTreeNodeFlags_DefaultOpen)) */
/* 	{ */
/* 	    ImGui::Text("mX/mY: %d / %d", (int32)InputState->MouseEvent->PosX, (int32)InputState->MouseEvent->PosY); */
/* 	    ImGui::Separator(); */
/* 	} */
/*     } */

/* static void RendererSettingsCollapseHeader(renderer *Renderer) */
/* { */
/*     if (ImGui::CollapsingHeader("Render settings", ImGuiTreeNodeFlags_DefaultOpen)) */
/*     { */
/* 	ImGui::Text("Memory Arena (bytes)"); */
/*         float progress = ((float)Renderer->MemoryArena->MaxUsed / (float)Renderer->MemoryArena->Size) * 1.0f; */
/*     	char buf[32]; */
/*         sprintf_s(buf, "%.1f/100 - %d/%d", 100 * progress, (int)(Renderer->MemoryArena->MaxUsed), (int)Renderer->MemoryArena->Size); */
/*         ImGui::ProgressBar(progress, ImVec2(-1.0f,0.f), buf); */
/*     	ImGui::Separator(); */

/*     	ImGui::Text("maxCube: %d", globalMaxCubeCount); */
/*     	ImGui::Text("current: %d", Renderer->Stats.CubeCount); */
/*     	ImGui::Text("draw: %d", Renderer->Stats.DrawCount); */
/*     	ImGui::Separator(); */
/*     } */
/* } */

// TODO: move to GUI
// void DrawSettingsPanel(engine *Engine,
// 		       uint32 width, uint32 height,
// 		       input_state *InputState,
// 		       camera *Camera,
// 		       renderer *Renderer,
// 		       uint32 &mapSize,
// 		       std::unordered_map<uint32, entity_cube> &containers,
// 		       std::unordered_map<uint32, uint32> &slots,
// 		       bool &focus)
// {
//     ImGui::SetNextWindowPos(ImVec2(10, 10));
//     ImGui::SetNextWindowSize(ImVec2(410, (float32)height - 20));
//     ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_NoResize);

//     EngineSettingsCollapseHeader(Engine, width, height);
//     input::InputStateSettingsCollapseHeader(InputState);
//     CameraSettingsCollapseHeader(Camera);
//     RendererSettingsCollapseHeader(Renderer);

//     // World
//     if (ImGui::CollapsingHeader("World settings", ImGuiTreeNodeFlags_DefaultOpen))
//     {
// 	ImGui::Text("slots: %03d/%03d", 0, slots.size());
// 	if (ImGui::SliderInt("floor", &(int)mapSize, 0, 1000))
// 	    slots.clear();
// 	ImGui::Separator();
//     }

//     // Containers
//     if (ImGui::CollapsingHeader("Object settings", ImGuiTreeNodeFlags_DefaultOpen))
//     {
// 	static uint32 selected = 0;
// 	ImGui::BeginChild("left pane", ImVec2(120, 150));
// 	for (std::pair<uint32, entity_cube> element : containers)	 
// 	{
// 	    char label[128];
// 	    sprintf_s(label, "obj: <%s>", element.second.Name);
// 	    if (ImGui::Selectable(label, (uint32)g_ContainerSelectedID == element.first))
// 		g_ContainerSelectedID = element.first;
// 	}

// 	ImGui::EndChild();
// 	ImGui::SameLine();

// 	ImGui::BeginChild("right pane", ImVec2(0, 150));
// 	if (g_ContainerSelectedID != 0)
// 	{
// 	    ImGui::Text("mem: %p", &containers[g_ContainerSelectedID]);
// 	    ImGui::Text("ID: %03d", containers[g_ContainerSelectedID].ID);
// 	    ImGui::Text("Name: %s", containers[g_ContainerSelectedID].Name);
// 	    ImGui::Text("State: %s",
// 			(containers[g_ContainerSelectedID].State == ENTITY_STATE_STATIC ? "STATIC" : "DYNAMIC"));
// 	    ImGui::Text("Pos x=%.2f y=%.2f z=%.2f",
// 			containers[g_ContainerSelectedID].Position.x,
// 			containers[g_ContainerSelectedID].Position.y,
// 			containers[g_ContainerSelectedID].Position.z);

// 	    ImGui::Text("Size x=%.2f y=%.2f z=%.2f w=%.2f",
// 			containers[g_ContainerSelectedID].Size.x,
// 			containers[g_ContainerSelectedID].Size.y,
// 			containers[g_ContainerSelectedID].Size.z,
// 			containers[g_ContainerSelectedID].Color.w);

// 	    ImGui::Text("Color r=%.2f g=%.2f b=%.2f a=%.2f",
// 			containers[g_ContainerSelectedID].Color.r,
// 			containers[g_ContainerSelectedID].Color.g,
// 			containers[g_ContainerSelectedID].Color.b,
// 			containers[g_ContainerSelectedID].Color.a);
// 	}

// 	ImGui::EndChild();
// 	ImGui::Separator();
//     }

//     // Slots
//     static uint32 selectedSlot = 0;
//     if (ImGui::CollapsingHeader("Slots settings", ImGuiTreeNodeFlags_DefaultOpen))
//     {
// 	ImGui::Columns(2);
// 	for (std::pair<uint32, uint32> sl : slots)
// 	{
// 	    char buf1[32];
// 	    sprintf_s(buf1, "%03d", sl.first);
// 	    ImGui::Button(buf1, ImVec2(-FLT_MIN, 0.0f));
// 	    ImGui::NextColumn();

// 	    char buf2[32];
// 	    if (sl.second == 0)
// 		sprintf_s(buf2, "%03d<empty>", sl.first);
// 	    else
// 		sprintf_s(buf2, "%03d", sl.second);

// 	    if (ImGui::Button(buf2, ImVec2(-FLT_MIN, 0.0f)))
// 	    {
// 		selectedSlot = sl.first;
// 		ImGui::OpenPopup("objects_popup");
// 	    }

// 	    ImGui::NextColumn();
// 	}

// 	ImGui::Columns(1);
// 	ImGui::Separator();
//     }

//     if (ImGui::BeginPopup("objects_popup"))
//     {
// 	ImGui::Text("slot-%03d", selectedSlot);
// 	ImGui::Separator();
// 	for (std::pair<uint32, entity_cube> ct : containers)	 
// 	{
// 	    if (ImGui::Selectable(containers[ct.first].Name))
// 		AttribContainerToSlot(slots, containers, selectedSlot, ct.first);
// 	}
// 	ImGui::EndPopup();
//     }
    
//     if (ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow))
// 	focus = true;
//     else
// 	focus = false;
//     ImGui::End();
// }
