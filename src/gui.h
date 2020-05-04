#pragma once

static void window_settings_collapse_header(window_t *Window, input_t *InputState);
static void editor_grid_collapse_header(uint32 &resolution, uint32 gridMaxResolution);
static void object_list_collapse_header(std::map<uint32, object_t*> *objects, uint32 *selectedObject, float32 pickingSphereRadius);
static void camera_settings_collapse_header(camera_t *Camera);

const float32 f32_zero = 0.1f, f32_two = 2.0f, f32_360 = 360.0f;

OPENFILENAME g_Ofn;
char g_szFile[260];
HWND g_hwnd;

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

	// Fonts
	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->AddFontDefault();
	static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
	ImFontConfig icons_config; icons_config.MergeMode = true; icons_config.PixelSnapH = true;
	//io.Fonts->AddFontFromFileTTF( "../assets/fonts/Font-Awesome-master/Font-Awesome-master/webfonts/fa-regular-400.ttf", 16.0f, &icons_config, icons_ranges );
	io.Fonts->AddFontFromFileTTF( "../assets/fonts/Font-Awesome-master/Font-Awesome-master/webfonts/fa-solid-900.ttf", 16.0f, &icons_config, icons_ranges );

        // Initialize OPENFILENAME
	ZeroMemory(&g_Ofn, sizeof(g_Ofn));
	g_Ofn.lStructSize = sizeof(g_Ofn);
	g_Ofn.hwndOwner = g_hwnd;
	g_Ofn.lpstrFile = g_szFile;
	g_Ofn.lpstrFile[0] = '\0';
	g_Ofn.nMaxFile = sizeof(g_szFile);
	g_Ofn.lpstrFilter = "All\0*.*\0Text\0*.TXT\0";
	g_Ofn.nFilterIndex = 1;
	g_Ofn.lpstrFileTitle = NULL;
	g_Ofn.nMaxFileTitle = 0;
	g_Ofn.lpstrInitialDir = NULL;
	g_Ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
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
    void ShowWindowStatsOverlay(window_t *Window, renderer_t *Renderer)
    {
	ImGui::SetNextWindowPos(ImVec2((float32)Window->Width - 210, 10));
	ImGui::SetNextWindowBgAlpha(0.35f);
	if (ImGui::Begin("window_stats", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav))
	{
	    ImGui::Text(Window->APIinfo.Vendor);
	    ImGui::Text(Window->APIinfo.Renderer);
	    ImGui::Text(Window->APIinfo.Version);
	    ImGui::Separator();
	    ImGui::Text("ms/f: %.3fms", Window->Time->MsPerFrame);
	    //ImGui::Text("fps: %d", Window->Time->FPS);
	    ImGui::Text("mcy/f: %d", Window->Time->MegaCyclePerFrame);
	    ImGui::Text("drawCalls: %d", Renderer->Stats.DrawCalls);
	    ImGui::End();
	}
    }

    // TODO: split panel
    void ShowEditorPanel(window_t *Window,
			 input_t *InputState,
			 camera_t *Camera,
			 uint32 &gridResolution,
			 uint32 gridMaxResolution,
			 std::map<uint32, object_t*> *objects,
			 uint32 *selectedObject,
			 float32 pickingSphereRadius,
			 bool &focus)
    {
	ImGui::SetNextWindowPos(ImVec2(10, 10));
	ImGui::SetNextWindowSize(ImVec2(410, (float32)Window->Height - 20));
	ImGui::Begin("settings", nullptr, ImGuiWindowFlags_NoResize);

        window_settings_collapse_header(Window, InputState);
	editor_grid_collapse_header(gridResolution, gridMaxResolution - 2);
	camera_settings_collapse_header(Camera);
        object_list_collapse_header(objects, selectedObject, pickingSphereRadius);
    
	if (ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow))
	    focus = true;
	else
	    focus = false;
	ImGui::End();
    }
}

static void window_settings_collapse_header(window_t *Window, input_t *InputState)
{
    if (ImGui::CollapsingHeader("Window settings", ImGuiTreeNodeFlags_DefaultOpen))
    {
    	ImVec2 bSize(40, 20);
	ImGui::Text("SCREEN: %d x %d", Window->Width, Window->Height);
	ImGui::Text("MousePos: %d x %d",
		    (uint32)InputState->MouseEvent->PosX,
		    (uint32)InputState->MouseEvent->PosY);
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

static void editor_grid_collapse_header(uint32 &resolution, uint32 gridMaxResolution)
{
    if (ImGui::CollapsingHeader("Grid", ImGuiTreeNodeFlags_DefaultOpen))
    {
	ImGui::SliderInt("res", &(int)resolution, 0, gridMaxResolution);
	ImGui::SameLine();
	ImGui::Text("max: %dx%d", gridMaxResolution, gridMaxResolution);
	ImGui::Separator();
    }
}

static void object_list_collapse_header(std::map<uint32, object_t*> *Scene,
					uint32 *selectedObject,
					float32 pickingSphereRadius)
{
    if (ImGui::CollapsingHeader("Object list", ImGuiTreeNodeFlags_DefaultOpen))
    {
	if (ImGui::Button("add"))
	{
	    if (GetOpenFileName(&g_Ofn)==TRUE)
	    {
		uint32 id = (uint32)Scene->size() + 1;
		model_t *loadedModel = model::LoadFromFile(g_szFile);
		object_t *obj = new object_t;
		obj->Model = loadedModel;
		obj->PickingSphere =  mesh::CreatePrimitiveSphereMesh(0.0f, pickingSphereRadius, 15, 15);

		Scene->insert({id, obj});
	    }
	}
	ImGui::Separator();

	static uint32 selected = 0;
	ImGui::BeginChild("left pane", ImVec2(120, 150));
	ImGui::Dummy(ImVec2(0.0f, 3.0f));	
	for (auto it = Scene->begin(); it != Scene->end(); it++)	 
	{
	    ImGui::Text( ICON_FA_CUBE );
	    ImGui::SameLine();
	    char label[128];
	    sprintf_s(label, "%d_%s", it->first, it->second->Model->ObjFilename.c_str());
	    if (ImGui::Selectable(label, *selectedObject == it->first))
	        *selectedObject = it->first;
	}

	ImGui::EndChild();
	ImGui::SameLine();

	ImGui::BeginChild("right pane", ImVec2(0, 150));
	if (*selectedObject != 0)
	{
	    ImGui::Text("mem: %p", &(*Scene)[*selectedObject]);
	    ImGui::Text("ID: %03d", *selectedObject);
	    ImGui::Text("Label: %s", (*Scene)[*selectedObject]->Model->ObjFilename.c_str());
	    ImGui::Text("Filepath: %s", (*Scene)[*selectedObject]->Model->Directory.c_str());
	    ImGui::Text("Pos x=%.2f y=%.2f z=%.2f",
		        (*Scene)[*selectedObject]->Position.x,
		        (*Scene)[*selectedObject]->Position.y,
		        (*Scene)[*selectedObject]->Position.z);
	    ImGui::SliderScalar("scale", ImGuiDataType_Float,
	    			&(*Scene)[*selectedObject]->Scale,
	    			&f32_zero, &f32_two);
	    ImGui::SliderScalar("rotate", ImGuiDataType_Float,
	    			&(*Scene)[*selectedObject]->Rotate,
	    			&f32_zero, &f32_360);

	    if (ImGui::Button("delete"))
	    {
		ImGui::OpenPopup("Delete?");
	    }
	}

	if (ImGui::BeginPopupModal("Delete?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("object will be deleted.\nThis operation cannot be undone!\n\n");
            ImGui::Separator();

            if (ImGui::Button("OK", ImVec2(120, 0))) {
		object::Delete((*Scene)[*selectedObject]);
		(*Scene).erase(*selectedObject);
		*selectedObject = 0;
		ImGui::CloseCurrentPopup();
	    }

            ImGui::SetItemDefaultFocus();
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0)))
		ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }

	ImGui::EndChild();
	ImGui::Separator();

	if (ImGui::Button("open"))
	{
	    if (GetOpenFileName(&g_Ofn)==TRUE)
	    {
		scene::OpenTextFormat(g_szFile, Scene);
	    }
	}
	ImGui::SameLine();

	if (ImGui::Button("save"))
	{
	    if (GetSaveFileName(&g_Ofn)==TRUE)
	    {
		scene::SaveTextFormat(g_szFile, Scene);
	    }
	}
	ImGui::Separator();
    }
}

static void camera_settings_collapse_header(camera_t *Camera)
{
    if (ImGui::CollapsingHeader("Camera settings", ImGuiTreeNodeFlags_DefaultOpen))
    {
	ImGui::Text("yaw: %.2f", Camera->Settings->Yaw);
	ImGui::Text("pitch: %.2f", Camera->Settings->Pitch);
	ImGui::Text("speed: %.2f", Camera->Settings->Speed);
	ImGui::Text("sensitivity: %.2f", Camera->Settings->Sensitivity);
	ImGui::Text("fov: %.2f", Camera->Settings->Fov);
	ImGui::Text("pos: %.2f, %.2f, %.2f",
		    Camera->Position.x,
		    Camera->Position.y,
		    Camera->Position.z);
	ImGui::Text("worldup: %.2f, %.2f, %.2f",
		    Camera->WorldUp.x,
		    Camera->WorldUp.y,
		    Camera->WorldUp.z);

	ImVec2 bSize(100, 20);
	if (ImGui::Button("Reset Default", bSize))
	{
	    Camera->Position = glm::vec3(0.0f, 5.0f, 10.0f);
	    Camera->Settings->Yaw = -90.0f;
	    Camera->Settings->Pitch = 0.0f;
	    Camera->Settings->Fov = 45.0f;
	    camera::ProcessMovementAngles(Camera, 0.0f, 0.0f);
	}
	ImGui::SameLine();
	if (ImGui::Button("Reset Up", bSize))
	{
	    Camera->Position = glm::vec3(0.0f, 30.0f, 0.0f);
	    Camera->Settings->Yaw = -90.0f;
	    Camera->Settings->Pitch = -90.0f;
	    Camera->Settings->Fov = 45.0f;
	    camera::ProcessMovementAngles(Camera, 0.0f, 0.0f);
	}
	ImGui::Separator();
    }
}