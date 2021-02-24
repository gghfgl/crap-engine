#pragma once

struct GameGui {
    GameGui(Window *window);
    ~GameGui();
    void NewFrame();
    void Draw();

    void PerformanceInfoOverlay(Renderer *renderer, PlateformInfo *info);

    void MakePanel(float32 posX, float32 posY);
    void EndPanel();
    void GeneralSettings(GameState *gs);

    bool activeWindow;
    Window *window;

private:
    ImVec2 dialogMaxSize;
    ImVec2 dialogMinSize;
};

GameGui::GameGui(Window* window)
{
    const char* glsl_version = "#version 450";
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window->context, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    this->window = window;
    this->dialogMaxSize = ImVec2((float)window->GetWidth(), (float)window->GetHeight());
    this->dialogMinSize = ImVec2(500.f, 250.f);
}

GameGui::~GameGui()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void GameGui::NewFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void GameGui::Draw()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

// ================================================================

void GameGui::MakePanel(float32 posX, float32 posY)
{
    ImGui::SetNextWindowPos(ImVec2(posX, posY));
    ImGui::SetNextWindowSize(ImVec2((float32)(this->window->GetWidth() / 5), (float32)(this->window->GetHeight() / 5)));
    ImGui::Begin("settings", nullptr, ImGuiWindowFlags_NoResize);
}

void GameGui::EndPanel()
{
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow))
        this->activeWindow = true;
    else
        this->activeWindow = false;
    ImGui::End();
}

// TODO: switch to string rendering with param window->time + renderer->stats + plateform->info
void GameGui::PerformanceInfoOverlay(Renderer *renderer, PlateformInfo *info)
{
    ImGui::SetNextWindowPos(ImVec2((float32)(this->window->GetWidth() - 210), 10));
    //ImGui::SetNextWindowBgAlpha(0.35f);
    if (ImGui::Begin("perf_stats", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoBackground))
    {
        ImGui::Text(info->vendor);
        ImGui::Text(info->graphicAPI);
        ImGui::Text(info->versionAPI);

        //ImGui::Separator();
        ImGui::Text("ms/f: %.3fms", this->window->time->deltaTime * 1000);
        //ImGui::Text("fps: %d", Window->time->FPS);
        //ImGui::Text("mcy/f: %d", Window->time->megaCyclePerFrame);
        ImGui::Text("drawCalls: %d", renderer->stats.drawCalls);
        ImGui::End();
    }
}

void GameGui::GeneralSettings(GameState *gs)
{
    // Window and input data
    ImGui::Text("SCREEN: %d x %d", this->window->GetWidth(), this->window->GetHeight());

    ImGui::Separator();
    ImGui::Dummy(ImVec2(0.0f, 3.0f));

    // Vsync toggle
    bool vsync = this->window->GetVsync();

    ImGui::PushStyleColor(ImGuiCol_Button,
                          vsync
                          ? (ImVec4)ImColor::HSV(0.3f, 1.0f, 0.6f)
                          : (ImVec4)ImColor::HSV(0.0f, 1.0f, 0.6f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                          vsync
                          ? (ImVec4)ImColor::HSV(0.3f, 1.0f, 0.8f)
                          : (ImVec4)ImColor::HSV(0.0f, 1.0f, 0.8f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                          vsync
                          ? (ImVec4)ImColor::HSV(0.3f, 1.0f, 0.8f)
                          : (ImVec4)ImColor::HSV(0.0f, 1.0f, 0.8f));

        
    ImVec2 bSize(100, 20);
    if (ImGui::Button("vsync", bSize))
        this->window->ToggleVsync();
    ImGui::PopStyleColor(3);

    // BoundingBox toggle
    ImGui::PushStyleColor(ImGuiCol_Button,
                          gs->showBoundingBox
                          ? (ImVec4)ImColor::HSV(0.3f, 1.0f, 0.6f)
                          : (ImVec4)ImColor::HSV(0.0f, 1.0f, 0.6f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                          gs->showBoundingBox
                          ? (ImVec4)ImColor::HSV(0.3f, 1.0f, 0.8f)
                          : (ImVec4)ImColor::HSV(0.0f, 1.0f, 0.8f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                          gs->showBoundingBox
                          ? (ImVec4)ImColor::HSV(0.3f, 1.0f, 0.8f)
                          : (ImVec4)ImColor::HSV(0.0f, 1.0f, 0.8f));

        
    if (ImGui::Button("bbox", bSize))
        gs->showBoundingBox = !gs->showBoundingBox;
    ImGui::PopStyleColor(3);
}
