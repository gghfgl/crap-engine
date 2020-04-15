#pragma once

/* void init_imgui(GLFWwindow* window) */
/* { */
/*     const char* glsl_version = "#version 450";  */
/*     IMGUI_CHECKVERSION(); */
/*     ImGui::CreateContext(); */
/*     //ImGuiIO& io = ImGui::GetIO(); (void)io; */
/*     //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls */
/*     ImGui::StyleColorsDark(); */
/*     ImGui_ImplGlfw_InitForOpenGL(window, true); */
/*     ImGui_ImplOpenGL3_Init(glsl_version); */
/* } */

/* void delete_imgui() */
/* { */
/*     ImGui_ImplOpenGL3_Shutdown(); */
/*     ImGui_ImplGlfw_Shutdown(); */
/*     ImGui::DestroyContext(); */
/* } */

/* void WrapImGuiNewFrame() */
/* { */
/*     ImGui_ImplOpenGL3_NewFrame(); */
/*     ImGui_ImplGlfw_NewFrame(); */
/*     ImGui::NewFrame(); */
/* } */

/* void WrapImGuiRender() */
/* { */
/*     ImGui::Render(); */
/*     ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());     */
/* } */

/* // TODO: move to GUI */
/* void EngineShowOverlay(engine *Engine) */
/* { */
/*     ImGuiIO& io = ImGui::GetIO(); // TODO: move to engine widow get width */
/*     ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - 210, 10)); */
/*     ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background */
/*     if (ImGui::Begin("Debug overlay", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav))    */
/*     { */
/* 	ImGui::Text("Debug overlay"); */
/* 	ImGui::Separator(); */
/* 	ImGui::Text(Engine->GPUModel); */
/* 	ImGui::Text(Engine->OpenglVersion); */
/* 	ImGui::Separator(); */
/* 	ImGui::Text("ms/f: %.3fms", Engine->Time->MsPerFrame); */
/* 	ImGui::Text("fps: %d", Engine->Time->FPS); */
/* 	ImGui::Text("mcy/f: %d", Engine->Time->MegaCyclePerFrame); */
/* 	ImGui::End(); */
/*     } */
/* } */

/* // TODO: move to GUI */
/* static void EngineSettingsCollapseHeader(engine *Engine, int width, int height) */
/* { */
/*     if (ImGui::CollapsingHeader("Engine settings", ImGuiTreeNodeFlags_DefaultOpen)) */
/*     { */
/*     	ImVec2 bSize(40, 20); */
/* 	ImGui::Text("screen: %d x %d", width, height); */
/* 	ImGui::Separator(); */

/* 	ImGui::PushID(1); */
/* 	if (ImGui::Button(Engine->Vsync ? "on" : "off", bSize)) */
/* 	    EngineToggleVsync(Engine); */
/* 	ImGui::SameLine(); */
/* 	ImGui::Text("VSYNC: "); */
/* 	ImGui::PopID(); */

/* 	ImGui::PushID(2); */
/* 	if (ImGui::Button(Engine->DebugMode ? "on" : "off", bSize)) */
/* 	    EngineToggleDebugMode(Engine); */
/* 	ImGui::SameLine(); */
/* 	ImGui::Text("DEBUG: "); */
/* 	ImGui::PopID(); */

/* 	ImGui::PushID(3); */
/* 	if (ImGui::Button(Engine->PolyMode ? "on" : "off", bSize)) */
/* 	    EngineTogglePolyMode(Engine); */
/* 	ImGui::SameLine(); */
/* 	ImGui::Text("POLY: "); */
/* 	ImGui::PopID(); */
/* 	ImGui::Separator(); */
/*     } */
/* } */

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
