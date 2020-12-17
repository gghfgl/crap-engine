#pragma once

#include "IMGUI/imgui.h"
#include "IMGUI/imgui_impl_glfw.h"
#include "IMGUI/imgui_impl_opengl3.h"
#include "igfd/ImGuiFileDialog.h"

#include "FONTAWESOME/IconsFontAwesome5.h"

struct EditorGui
{
    EditorGui(Window *window);
    ~EditorGui();
    void newFrame();
    void draw();

    void performanceInfoOverlay(Renderer *renderer, PlateformInfo *info, GlobalState *gs);
    void progressPlotWidget(bool animate);

    void makePanel(float32 posX, float32 posY);
    void endPanel();

    void windowAndInputSettings(InputState *input);
    void cameraSettings(Camera *camera);
    void groundSettings(uint32 &currentGroundIndex,
                        std::map<uint32, Ground*> *Grounds,
                        uint32 maxResolution);
    void skyboxSettings(uint32 &currentSkyboxIndex,
                        std::map<uint32, Skybox*> *Skyboxes);
    void environmentSettings(Ground *ground,
                             int32 &resolution,
                             uint32 maxResolution,
                             bool *showSkybox);

    void moduleSettings(uint32 &currentModuleIndex,
                        std::map<uint32, Module*> *Modules); // TODO: pickingSphereRadius?
    /* void entitiesSettings(std::map<uint32, Entity*> *Scene, */
    /*                       uint32 *selectedEntity, */
    /*                       float32 pickingSphereRadius); */
    
    bool activeWindow;
    Window *window;
    const float32 f32_zero = 0.1f, f32_two = 2.0f, f32_ten = 10.0f, f32_360 = 360.0f;

private:
    ImVec2 dialogMaxSize;
    ImVec2 dialogMinSize;
};

EditorGui::EditorGui(Window* window)
{
    const char* glsl_version = "#version 450";
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    //ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window->context, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Fonts
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontDefault();
    static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
    ImFontConfig icons_config; icons_config.MergeMode = true; icons_config.PixelSnapH = true;
    io.Fonts->AddFontFromFileTTF( "assets/fonts/webfonts/fa-solid-900.ttf", 13.0f, &icons_config, icons_ranges );

    this->activeWindow = false;
    this->window = window;
    this->dialogMaxSize = ImVec2((float)window->getWidth(), (float)window->getHeight());
    this->dialogMinSize = ImVec2(500.f, 250.f);
}

EditorGui::~EditorGui()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void EditorGui::newFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void EditorGui::draw()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

// ================================================================

// TODO: switch to string rendering with param window->time + renderer->stats + plateform->info
void EditorGui::performanceInfoOverlay(Renderer *renderer,
                                       PlateformInfo *info,
                                       GlobalState *gs)
{
    ImGui::SetNextWindowPos(ImVec2((float32)(this->window->getWidth() - 210), 10));
    ImGui::SetNextWindowBgAlpha(0.35f);
    if (ImGui::Begin("perf_stats", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav))
    {
        ImGui::Text(info->vendor);
        ImGui::Text(info->graphicAPI);
        ImGui::Text(info->versionAPI);

        ImGui::Separator();
        ImGui::Text("ms/f: %.3fms", this->window->time->deltaTime * 1000);
        //ImGui::Text("fps: %d", Window->time->FPS);
        //ImGui::Text("mcy/f: %d", Window->time->megaCyclePerFrame);
        ImGui::Text("drawCalls: %d", renderer->stats.drawCalls);

        ImGui::Separator();
        ImGui::Text("w_active: %d", this->activeWindow);
        ImGui::Text("c_ground: %d", gs->currentGroundIndex);
        ImGui::Text("c_skybox: %d", gs->currentSkyboxIndex);
        ImGui::Text("c_module: %d", gs->currentModuleIndex);
        ImGui::Text("m_hover: %d", gs->hoveredModule);
        ImGui::Text("m_select: %d", gs->selectedModule);
        ImGui::Text("m_drag: %d", gs->dragModule);
        ImGui::End();
    }
}

void EditorGui::makePanel(float32 posX, float32 posY)
{
    ImGui::SetNextWindowPos(ImVec2(posX, posY));
    ImGui::SetNextWindowSize(ImVec2(410, (float32)(this->window->getHeight() - 20)));
    ImGui::Begin("settings", nullptr, ImGuiWindowFlags_NoResize);
}

void EditorGui::endPanel()
{
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow))
        this->activeWindow = true;
    else
        this->activeWindow = false;
    ImGui::End();
}

// =================================================

void EditorGui::windowAndInputSettings(InputState *input)
{
    if (ImGui::CollapsingHeader("Window & Input", ImGuiTreeNodeFlags_DefaultOpen))
    {
        // Window and input data
        ImGui::Text("SCREEN: %d x %d", this->window->getWidth(), this->window->getHeight());
        ImGui::Text("MousePos: %d x %d",
                    (uint32)input->mouse->posX,
                    (uint32)input->mouse->posY);

        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0f, 3.0f));

        // Vsync toggle
        bool vsync = this->window->getVsync();

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
        if (ImGui::Button(vsync ? "on" : "off", bSize))
            this->window->toggleVsync();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGui::Text("VSYNC");
    }

    ImGui::Separator();
}

void EditorGui::cameraSettings(Camera *camera)
{
    if (ImGui::CollapsingHeader("camera", ImGuiTreeNodeFlags_DefaultOpen))
    {
        // Camera data
        ImGui::Text("yaw: %.2f", camera->settings->yaw);
        ImGui::Text("pitch: %.2f", camera->settings->pitch);
        ImGui::Text("speed: %.2f", camera->settings->speed);
        ImGui::Text("sensitivity: %.2f", camera->settings->sensitivity);
        ImGui::Text("fov: %.2f", camera->settings->fov);
        ImGui::Text("pos: %.2f, %.2f, %.2f",
                    camera->position.x,
                    camera->position.y,
                    camera->position.z);
        ImGui::Text("worldup: %.2f, %.2f, %.2f",
                    camera->worldUp.x,
                    camera->worldUp.y,
                    camera->worldUp.z);

        // Reset bouton
        ImVec2 bSize(100, 20);
        if (ImGui::Button("Reset Default", bSize))
        {
            camera->position = glm::vec3(0.0f, 5.0f, 10.0f);
            camera->settings->yaw = -90.0f;
            camera->settings->pitch = 0.0f;
            camera->settings->fov = 45.0f;
            camera->processMovementAngles(0.0f, 0.0f);
        }

        // Reset button
        ImGui::SameLine();
        if (ImGui::Button("Reset Up", bSize))
        {
            camera->position = glm::vec3(0.0f, 30.0f, 0.0f);
            camera->settings->yaw = -90.0f;
            camera->settings->pitch = -90.0f;
            camera->settings->fov = 45.0f;
            camera->processMovementAngles(0.0f, 0.0f);
        }

        ImGui::Separator();
    }
}

void EditorGui::groundSettings(uint32 &currentGroundIndex,
                               std::map<uint32, Ground*> *Grounds,
                               uint32 maxResolution)
{
    if (ImGui::CollapsingHeader("ground", ImGuiTreeNodeFlags_DefaultOpen))
    {
        // Add Ground
        ImGui::Dummy(ImVec2(0.0f, 3.0f));	
        if (ImGui::Button("add##ground"))
            ImGui::OpenPopup("AddGround");                    

        if (ImGui::BeginPopupModal("AddGround"))
        {
            ImGui::Dummy(ImVec2(0.0f, 3.0f));	
            ImGui::Text("add a new ground to the list:");
            static char str0[32] = "give me a name";
            ImGui::PushItemWidth(-FLT_MIN);
            ImGui::InputText("", str0, IM_ARRAYSIZE(str0));
            ImGui::Separator();

            ImGui::Dummy(ImVec2(0.0f, 3.0f));
            if (ImGui::Button("Add##ground", ImVec2(120, 0)))
            {
                char *name = new char[32];
                strncpy(name, str0, 32);
                name[32 - 1] = '\0';
                
                Ground *ground = new Ground(name, "");
                std::time_t timestamp = std::time(nullptr);
                Grounds->insert({static_cast<uint32>(timestamp), ground});

                // Reset input placeholder
                strncpy(str0, "give me a name", 32);
                
                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();
            if (ImGui::Button("Close##ground", ImVec2(120, 0)))
                ImGui::CloseCurrentPopup();

            ImGui::EndPopup();
        }

        int32 error = 0;
        // Save Ground list into file
        ImGui::SameLine();
        if (ImGui::Button("save##ground"))
        {
            std::time_t timestamp = std::time(nullptr);

            // Set middleish position for every nex windows
            ImGui::SetNextWindowPos(ImVec2(this->window->getWidth() / 2.5, this->window->getHeight() / 2.5));
            igfd::ImGuiFileDialog::Instance()->OpenDialog("SaveGroundListInTextFormat", "Choose File", ".list", ".", "crap_grounds_"+std::to_string(timestamp));
        }

        if (igfd::ImGuiFileDialog::Instance()->FileDialog("SaveGroundListInTextFormat", ImGuiWindowFlags_NoCollapse, this->dialogMinSize, this->dialogMaxSize))
        {
            if (igfd::ImGuiFileDialog::Instance()->IsOk == true)
            {
                std::string filePathName = igfd::ImGuiFileDialog::Instance()->GetFilePathName();
                error = SaveGroundListInTextFormat(filePathName.c_str(), Grounds);
            }

            igfd::ImGuiFileDialog::Instance()->CloseDialog("SaveGroundListInTextFormat");
        }

        // Open Ground list from file
        ImGui::SameLine();
        if (ImGui::Button("open##ground"))
        {
            // Set middleish position for every nex windows
            ImGui::SetNextWindowPos(ImVec2(this->window->getWidth() / 2.5, this->window->getHeight() / 2.5));
            igfd::ImGuiFileDialog::Instance()->OpenDialog("OpenGroundListFromFile", "Choose File", ".list", ".");
            igfd::ImGuiFileDialog::Instance()->SetExtentionInfos(".list", ImVec4(0,1,0, 0.9));
        }

        if (igfd::ImGuiFileDialog::Instance()->FileDialog("OpenGroundListFromFile", ImGuiWindowFlags_NoCollapse, this->dialogMinSize, this->dialogMaxSize))
        {
            if (igfd::ImGuiFileDialog::Instance()->IsOk == true)
            {
                std::string filePathName = igfd::ImGuiFileDialog::Instance()->GetFilePathName();
                error = OpenGroundListFromFile(filePathName.c_str(), Grounds);
            }

            igfd::ImGuiFileDialog::Instance()->CloseDialog("OpenGroundListFromFile");
        }

        if (error != 0)
            ImGui::OpenPopup("GroundListError");

        if (ImGui::BeginPopupModal("GroundListError"))
        {
            error = 0;
            ImGui::Dummy(ImVec2(0.0f, 3.0f));	
            ImGui::Text("Error while saving/opening ground list file");
            ImGui::Separator();
            if (ImGui::Button("Close"))
                ImGui::CloseCurrentPopup();

            ImGui::EndPopup();
        }
        
        ImGui::Dummy(ImVec2(0.0f, 3.0f));	

        // Display Ground list
        for (auto it = Grounds->cbegin(), it_next = it; it != Grounds->cend(); it = it_next)
        {
            ++it_next; // used because  we deleting entry while looping through the map
            if (ImGui::TreeNode((void*)(intptr_t)it->first, "%s %s", it->second->name, (currentGroundIndex == it->first) ? "[selected]" : ""))
            {
                auto IDToString = std::to_string(it->first);
                ImGui::Dummy(ImVec2(0.0f, 3.0f));

                // Ground data
                ImGui::Text( ICON_FA_CUBE );
                ImGui::SameLine();
                if (it->second->entity->model == nullptr)
                    ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), "unknown");
                else
                    ImGui::Text("%s", it->second->entity->model->objFilename.c_str());

                ImGui::Text( ICON_FA_FOLDER );
                ImGui::SameLine();
                if (it->second->entity->model == nullptr)
                    ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), "unknown");
                else
                    ImGui::Text("%s", it->second->entity->model->directory.c_str());

                // Resolution slider
                //ImGui::PushItemWidth(-FLT_MIN);
                ImGui::Text( ICON_FA_EXPAND );
                ImGui::SameLine();
                ImGui::SliderInt("##resolution", &it->second->resolutionBuffer, 0, maxResolution);
                ImGui::SameLine();
                ImGui::Text("max %dx%d", maxResolution, maxResolution);

                ImGui::Text( ICON_FA_DATABASE );
                ImGui::SameLine();

                // Load Ground model
                if (ImGui::SmallButton("load"))
                {
                    // Set middleish position for every nex windows
                    ImGui::SetNextWindowPos(ImVec2(this->window->getWidth() / 2.5, this->window->getHeight() / 2.5));
                    igfd::ImGuiFileDialog::Instance()->OpenDialog("LoadGroundModel###"+IDToString, "Choose File", ".obj", ".");
                    igfd::ImGuiFileDialog::Instance()->SetExtentionInfos(".obj", ImVec4(1,1,0, 0.9));
                }

                if (igfd::ImGuiFileDialog::Instance()->FileDialog("LoadGroundModel###"+IDToString, ImGuiWindowFlags_NoCollapse, this->dialogMinSize, this->dialogMaxSize))
                {
                    if (igfd::ImGuiFileDialog::Instance()->IsOk == true)
                    {
                        std::string filePathName = igfd::ImGuiFileDialog::Instance()->GetFilePathName();

                        Model *loadedModel = new Model(filePathName);
                        if (loadedModel != nullptr)
                        {
                            delete it->second->entity->model;
                            it->second->entity->model = loadedModel;
                            it->second->isGenerated = false;
                        }
                    }

                    igfd::ImGuiFileDialog::Instance()->CloseDialog("LoadGroundModel###"+IDToString);
                }

                // Rename Ground
                ImGui::SameLine();
                if (ImGui::SmallButton("rename"))
                {
                    ImGui::OpenPopup("RenameGround");
                }

                if (ImGui::BeginPopupModal("RenameGround"))
                {
                    ImGui::Dummy(ImVec2(0.0f, 3.0f));	
                    ImGui::Text("rename ground [%s]:", it->second->name);
                    ImGui::PushItemWidth(-FLT_MIN);
                    static char str1[32] = "give me a name";
                    ImGui::InputText("", str1, IM_ARRAYSIZE(str1));
                    ImGui::Separator();

                    ImGui::Dummy(ImVec2(0.0f, 3.0f));
                    if (ImGui::Button("Ok", ImVec2(120, 0)))
                    {
                        char *rename = new char[32];
                        strncpy(rename, str1, 32);
                        rename[32 - 1] = '\0';

                        delete[] it->second->name;
                        it->second->name = rename;

                        // Reset input placeholder
                        strncpy(str1, "give me a name", 32);

                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Cancel", ImVec2(120, 0)))
                        ImGui::CloseCurrentPopup();

                    ImGui::EndPopup();
                }

                // Select Ground
                ImGui::SameLine();
                if (ImGui::SmallButton("select"))
                {
                    if (currentGroundIndex == it->first)
                        currentGroundIndex = 0;
                    else
                        currentGroundIndex = it->first;                    
                }

                // Delete Ground
                ImGui::SameLine();
                if (ImGui::SmallButton("delete"))
                    ImGui::OpenPopup("DeleteGround");
                
                if (ImGui::BeginPopupModal("DeleteGround"))
                {
                    ImGui::Text("selected ground will be deleted.\nThis operation cannot be undone!\n\n");
                    ImGui::Separator();

                    if (ImGui::Button("OK", ImVec2(120, 0))) {
                        if (it->first == currentGroundIndex)
                            currentGroundIndex = 0;

                        delete it->second;
                        Grounds->erase(it);

                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::SetItemDefaultFocus();
                    ImGui::SameLine();
                    if (ImGui::Button("Cancel", ImVec2(120, 0)))
                        ImGui::CloseCurrentPopup();
                    ImGui::EndPopup();
                }

                ImGui::Dummy(ImVec2(0.0f, 10.0f));
                ImGui::TreePop();
            }
        }
    }

    ImGui::Separator();
}

void EditorGui::skyboxSettings(uint32 &currentSkyboxIndex,
                               std::map<uint32, Skybox*> *Skyboxes)
{
    if (ImGui::CollapsingHeader("skybox", ImGuiTreeNodeFlags_DefaultOpen))
    {
        // Add Skybox
        ImGui::Dummy(ImVec2(0.0f, 3.0f));
        if (ImGui::Button("add##skybox"))
        {
            ImGui::OpenPopup("AddSkybox");
        }

        if (ImGui::BeginPopupModal("AddSkybox"))
        {
            ImGui::Dummy(ImVec2(0.0f, 3.0f));
            ImGui::Text("add a new skybox to the list:");
            static char str0[32] = "give me a name";
            ImGui::PushItemWidth(-FLT_MIN);
            ImGui::InputText("", str0, IM_ARRAYSIZE(str0));
            ImGui::Separator();

            ImGui::Dummy(ImVec2(0.0f, 3.0f));
            if (ImGui::Button("Add##skybox", ImVec2(120, 0)))
            {
                char *name = new char[32];
                strncpy(name, str0, 32);
                name[32 - 1] = '\0';

                Skybox *skybox = new Skybox(name, "");
                std::time_t timestamp = std::time(nullptr);
                Skyboxes->insert({static_cast<uint32>(timestamp), skybox});

                // Reset input placeholder
                strncpy(str0, "give me a name", 32);

                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();
            if (ImGui::Button("Close##skybox", ImVec2(120, 0)))
                ImGui::CloseCurrentPopup();

            ImGui::EndPopup();
        }

        int32 error = 0;
        // Save Skybox list into file
        ImGui::SameLine();
        if (ImGui::Button("save##skybox"))
        {
            std::time_t timestamp = std::time(nullptr);

            // Set middleish position for every nex windows
            ImGui::SetNextWindowPos(ImVec2(this->window->getWidth() / 2.5, this->window->getHeight() / 2.5));
            igfd::ImGuiFileDialog::Instance()->OpenDialog("SaveSkyboxListInTextFormat", "Choose File###skybox", ".list", ".", "crap_skyboxes_"+std::to_string(timestamp));
        }

        if (igfd::ImGuiFileDialog::Instance()->FileDialog("SaveSkyboxListInTextFormat", ImGuiWindowFlags_NoCollapse, this->dialogMinSize, this->dialogMaxSize))
        {
            if (igfd::ImGuiFileDialog::Instance()->IsOk == true)
            {
                std::string filePathName = igfd::ImGuiFileDialog::Instance()->GetFilePathName();
                error = SaveSkyboxListInTextFormat(filePathName.c_str(), Skyboxes);
            }

            igfd::ImGuiFileDialog::Instance()->CloseDialog("SaveSkyboxListInTextFormat");
        }

        // Open Skybox list from file
        ImGui::SameLine();
        if (ImGui::Button("open##skybox"))
        {
            // Set middleish position for every nex windows
            ImGui::SetNextWindowPos(ImVec2(this->window->getWidth() / 2.5, this->window->getHeight() / 2.5));
            igfd::ImGuiFileDialog::Instance()->OpenDialog("OpenSkyboxListFromFile", "Choose File###skybox", ".list", ".");
            igfd::ImGuiFileDialog::Instance()->SetExtentionInfos(".list", ImVec4(0,1,0, 0.9));
        }

        if (igfd::ImGuiFileDialog::Instance()->FileDialog("OpenSkyboxListFromFile", ImGuiWindowFlags_NoCollapse, this->dialogMinSize, this->dialogMaxSize))
        {
            if (igfd::ImGuiFileDialog::Instance()->IsOk == true)
            {
                std::string filePathName = igfd::ImGuiFileDialog::Instance()->GetFilePathName();
                error = OpenSkyboxListFromFile(filePathName.c_str(), Skyboxes);
            }

            igfd::ImGuiFileDialog::Instance()->CloseDialog("OpenSkyboxListFromFile");
        }

        if (error != 0)
            ImGui::OpenPopup("SkyboxListError");

        if (ImGui::BeginPopupModal("SkyboxListError"))
        {
            error = 0;
            ImGui::Dummy(ImVec2(0.0f, 3.0f));	
            ImGui::Text("Error while saving/opening skybox list file");
            ImGui::Separator();
            if (ImGui::Button("Close"))
                ImGui::CloseCurrentPopup();

            ImGui::EndPopup();
        }

        ImGui::Dummy(ImVec2(0.0f, 3.0f));

        // Display Skybox list
        for (auto it = Skyboxes->cbegin(), it_next = it; it != Skyboxes->cend(); it = it_next)
        {
            ++it_next;
            if (ImGui::TreeNode((void*)(intptr_t)it->first, "%s %s", it->second->name, (currentSkyboxIndex == it->first) ? "[selected]" : ""))
            {
                auto IDToString = std::to_string(it->first);
                ImGui::Dummy(ImVec2(0.0f, 3.0f));

                // Skybox data
                ImGui::Text( ICON_FA_FOLDER );
                ImGui::SameLine();
                if (it->second->directory.length() == 0)
                    ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), "unknown");
                else
                    ImGui::Text("%s", it->second->directory.c_str());

                ImGui::Text( ICON_FA_DATABASE );
                ImGui::SameLine();

                // Load Skybox images
                if (ImGui::SmallButton("load"))
                {
                    // Set middleish position for every nex windows
                    ImGui::SetNextWindowPos(ImVec2(this->window->getWidth() / 2.5, this->window->getHeight() / 2.5));
                    igfd::ImGuiFileDialog::Instance()->OpenDialog("LoadSkyboxModel###"+IDToString, "Choose File###skybox", ".jpg", ".");
                }

                if (igfd::ImGuiFileDialog::Instance()->FileDialog("LoadSkyboxModel###"+IDToString, ImGuiWindowFlags_NoCollapse, this->dialogMinSize, this->dialogMaxSize))
                {
                    if (igfd::ImGuiFileDialog::Instance()->IsOk == true)
                    {
                        std::string filePathName = igfd::ImGuiFileDialog::Instance()->GetCurrentPath();
                        it->second->loadCubeMapTextureFromFile(filePathName);
                    }

                    igfd::ImGuiFileDialog::Instance()->CloseDialog("LoadSkyboxModel###"+IDToString);
                }
                
                // Rename Skybox
                ImGui::SameLine();
                if (ImGui::SmallButton("rename"))
                {
                    ImGui::OpenPopup("RenameSkybox");
                }

                if (ImGui::BeginPopupModal("RenameSkybox"))
                {
                    ImGui::Dummy(ImVec2(0.0f, 3.0f));	
                    ImGui::Text("rename skybox [%s]:", it->second->name);
                    ImGui::PushItemWidth(-FLT_MIN);
                    static char str1[32] = "give me a name";
                    ImGui::InputText("", str1, IM_ARRAYSIZE(str1));
                    ImGui::Separator();

                    ImGui::Dummy(ImVec2(0.0f, 3.0f));
                    if (ImGui::Button("Ok", ImVec2(120, 0)))
                    {
                        char *rename = new char[32];
                        strncpy(rename, str1, 32);
                        rename[32 - 1] = '\0';

                        delete[] it->second->name;
                        it->second->name = rename;

                        // Reset input placeholder
                        strncpy(str1, "give me a name", 32);

                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Cancel", ImVec2(120, 0)))
                        ImGui::CloseCurrentPopup();

                    ImGui::EndPopup();
                }

                // Select Skybox
                ImGui::SameLine();
                if (ImGui::SmallButton("select"))
                {
                    if (currentSkyboxIndex == it->first)
                        currentSkyboxIndex = 0;
                    else
                        currentSkyboxIndex = it->first;                    
                }

                // Delete Skybox
                ImGui::SameLine();
                if (ImGui::SmallButton("delete"))
                    ImGui::OpenPopup("DeleteSkybox");
                
                if (ImGui::BeginPopupModal("DeleteSkybox"))
                {
                    ImGui::Text("selected skybox will be deleted.\nThis operation cannot be undone!\n\n");
                    ImGui::Separator();

                    if (ImGui::Button("OK", ImVec2(120, 0))) {
                        if (it->first == currentSkyboxIndex)
                            currentSkyboxIndex = 0;

                        delete it->second;
                        Skyboxes->erase(it);

                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::SetItemDefaultFocus();
                    ImGui::SameLine();
                    if (ImGui::Button("Cancel", ImVec2(120, 0)))
                        ImGui::CloseCurrentPopup();
                    ImGui::EndPopup();
                }

                ImGui::Dummy(ImVec2(0.0f, 10.0f));
                ImGui::TreePop();
            }
        }        
    }
    
    ImGui::Separator();
}

void EditorGui::moduleSettings(uint32 &currentModuleIndex,
                               std::map<uint32, Module*> *Modules)
{
    if (ImGui::CollapsingHeader("module", ImGuiTreeNodeFlags_DefaultOpen))
    {
        // Add Module
        ImGui::Dummy(ImVec2(0.0f, 3.0f));	
        if (ImGui::Button("add##module"))
            ImGui::OpenPopup("AddModule");                    

        if (ImGui::BeginPopupModal("AddModule"))
        {
            ImGui::Dummy(ImVec2(0.0f, 3.0f));	
            ImGui::Text("add a new module to the list:");
            static char str0[32] = "give me a name";
            ImGui::PushItemWidth(-FLT_MIN);
            ImGui::InputText("", str0, IM_ARRAYSIZE(str0));
            ImGui::Separator();

            ImGui::Dummy(ImVec2(0.0f, 3.0f));
            if (ImGui::Button("Add##module", ImVec2(120, 0)))
            {
                char *name = new char[32];
                strncpy(name, str0, 32);
                name[32 - 1] = '\0';
                
                Module *module = new Module(name, "");
                std::time_t timestamp = std::time(nullptr);
                Modules->insert({static_cast<uint32>(timestamp), module});
                
                // Reset input placeholder
                strncpy(str0, "give me a name", 32);
                
                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();
            if (ImGui::Button("Close##module", ImVec2(120, 0)))
                ImGui::CloseCurrentPopup();

            ImGui::EndPopup();
        }

        int32 error = 0;
        // Save Module list into file
        ImGui::SameLine();
        if (ImGui::Button("save##module"))
        {
            std::time_t timestamp = std::time(nullptr);

            // Set middleish position for every nex windows
            ImGui::SetNextWindowPos(ImVec2(this->window->getWidth() / 2.5, this->window->getHeight() / 2.5));
            igfd::ImGuiFileDialog::Instance()->OpenDialog("SaveModuleListInTextFormat", "Choose File", ".list", ".", "crap_modules_"+std::to_string(timestamp));
        }

        if (igfd::ImGuiFileDialog::Instance()->FileDialog("SaveModuleListInTextFormat", ImGuiWindowFlags_NoCollapse, this->dialogMinSize, this->dialogMaxSize))
        {
            if (igfd::ImGuiFileDialog::Instance()->IsOk == true)
            {
                std::string filePathName = igfd::ImGuiFileDialog::Instance()->GetFilePathName();
                error = SaveModuleListInTextFormat(filePathName.c_str(), Modules);
            }

            igfd::ImGuiFileDialog::Instance()->CloseDialog("SaveModuleListInTextFormat");
        }

        // Open Module list from file
        ImGui::SameLine();
        if (ImGui::Button("open##module"))
        {
            // Set middleish position for every nex windows
            ImGui::SetNextWindowPos(ImVec2(this->window->getWidth() / 2.5, this->window->getHeight() / 2.5));
            igfd::ImGuiFileDialog::Instance()->OpenDialog("OpenModuleListFromFile", "Choose File", ".list", ".");
            igfd::ImGuiFileDialog::Instance()->SetExtentionInfos(".list", ImVec4(0,1,0, 0.9));
        }

        if (igfd::ImGuiFileDialog::Instance()->FileDialog("OpenModuleListFromFile", ImGuiWindowFlags_NoCollapse, this->dialogMinSize, this->dialogMaxSize))
        {
            if (igfd::ImGuiFileDialog::Instance()->IsOk == true)
            {
                std::string filePathName = igfd::ImGuiFileDialog::Instance()->GetFilePathName();
                error = OpenModuleListFromFile(filePathName.c_str(), Modules);
            }

            igfd::ImGuiFileDialog::Instance()->CloseDialog("OpenModuleListFromFile");
        }

        if (error != 0)
            ImGui::OpenPopup("ModuleListError");

        if (ImGui::BeginPopupModal("ModuleListError"))
        {
            error = 0;
            ImGui::Dummy(ImVec2(0.0f, 3.0f));	
            ImGui::Text("Error while saving/opening module list file");
            ImGui::Separator();
            if (ImGui::Button("Close"))
                ImGui::CloseCurrentPopup();

            ImGui::EndPopup();
        }
        
        ImGui::Dummy(ImVec2(0.0f, 3.0f));	

        // Display Module list
        for (auto it = Modules->cbegin(), it_next = it; it != Modules->cend(); it = it_next)
        {
            ++it_next; // used because  we deleting entry while looping through the map
            if (ImGui::TreeNode((void*)(intptr_t)it->first, "%s %s", it->second->name, (currentModuleIndex == it->first) ? "[selected]" : ""))
            {
                auto IDToString = std::to_string(it->first);
                ImGui::Dummy(ImVec2(0.0f, 3.0f));

                // Module data
                ImGui::Text( ICON_FA_CUBE );
                ImGui::SameLine();
                if (it->second->entity->model == nullptr)
                    ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), "unknown");
                else
                    ImGui::Text("%s", it->second->entity->model->objFilename.c_str());

                ImGui::Text( ICON_FA_FOLDER );
                ImGui::SameLine();
                if (it->second->entity->model == nullptr)
                    ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), "unknown");
                else
                    ImGui::Text("%s", it->second->entity->model->directory.c_str());

                ImGui::Text( ICON_FA_MAP_MARKER_ALT );
                ImGui::SameLine();
                ImGui::Text("x=%.2f y=%.2f z=%.2f",
                            it->second->entity->position.x,
                            it->second->entity->position.y,
                            it->second->entity->position.z);

                // TODO: Show texture checkbox

                ImGui::Text( ICON_FA_EXTERNAL_LINK_ALT );
                ImGui::SameLine();
                ImGui::SliderScalar("##scale", ImGuiDataType_Float,
                                    &it->second->entity->scale,
                                    &f32_zero, &f32_ten);
                
                ImGui::Text( ICON_FA_SYNC );
                ImGui::SameLine();
                ImGui::SliderScalar("##rotate", ImGuiDataType_Float,
                                    &it->second->entity->rotate,
                                    &f32_zero, &f32_360);

                // TODO: Animation controls (start / stop / reset)
                
                ImGui::Text( ICON_FA_DATABASE );
                ImGui::SameLine();

                // Load Module
                if (ImGui::SmallButton("load"))
                {
                    // Set middleish position for every nex windows
                    ImGui::SetNextWindowPos(ImVec2(this->window->getWidth() / 2.5, this->window->getHeight() / 2.5));
                    igfd::ImGuiFileDialog::Instance()->OpenDialog("LoadModuleModel###"+IDToString, "Choose File", ".obj", ".");
                    igfd::ImGuiFileDialog::Instance()->SetExtentionInfos(".obj", ImVec4(1,1,0, 0.9));
                }

                if (igfd::ImGuiFileDialog::Instance()->FileDialog("LoadModuleModel###"+IDToString, ImGuiWindowFlags_NoCollapse, this->dialogMinSize, this->dialogMaxSize))
                {
                    if (igfd::ImGuiFileDialog::Instance()->IsOk == true)
                    {
                        std::string filePathName = igfd::ImGuiFileDialog::Instance()->GetFilePathName();

                        Model *loadedModel = new Model(filePathName);
                        if (loadedModel != nullptr)
                        {
                            delete it->second->entity->model;
                            it->second->entity->model = loadedModel;
                        }
                    }

                    igfd::ImGuiFileDialog::Instance()->CloseDialog("LoadModuleModel###"+IDToString);
                }

                // Rename Module
                ImGui::SameLine();
                if (ImGui::SmallButton("rename"))
                {
                    ImGui::OpenPopup("RenameModule");
                }

                if (ImGui::BeginPopupModal("RenameModule"))
                {
                    ImGui::Dummy(ImVec2(0.0f, 3.0f));	
                    ImGui::Text("rename module [%s]:", it->second->name);
                    ImGui::PushItemWidth(-FLT_MIN);
                    static char str1[32] = "give me a name";
                    ImGui::InputText("", str1, IM_ARRAYSIZE(str1));
                    ImGui::Separator();

                    ImGui::Dummy(ImVec2(0.0f, 3.0f));
                    if (ImGui::Button("Ok", ImVec2(120, 0)))
                    {
                        char *rename = new char[32];
                        strncpy(rename, str1, 32);
                        rename[32 - 1] = '\0';

                        delete[] it->second->name;
                        it->second->name = rename;

                        // Reset input placeholder
                        strncpy(str1, "give me a name", 32);

                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Cancel", ImVec2(120, 0)))
                        ImGui::CloseCurrentPopup();

                    ImGui::EndPopup();
                }

                // Select Module
                ImGui::SameLine();
                if (ImGui::SmallButton("select"))
                {
                    if (currentModuleIndex == it->first)
                        currentModuleIndex = 0;
                    else
                        currentModuleIndex = it->first;                    
                }

                // Delete Module
                ImGui::SameLine();
                if (ImGui::SmallButton("delete"))
                    ImGui::OpenPopup("DeleteModule");
                
                // Delete ground modal
                if (ImGui::BeginPopupModal("DeleteModule"))
                {
                    ImGui::Text("selected module will be deleted.\nThis operation cannot be undone!\n\n");
                    ImGui::Separator();

                    if (ImGui::Button("OK", ImVec2(120, 0))) {
                        if (it->first == currentModuleIndex)
                            currentModuleIndex = 0;

                        delete it->second;
                        Modules->erase(it);

                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::SetItemDefaultFocus();
                    ImGui::SameLine();
                    if (ImGui::Button("Cancel", ImVec2(120, 0)))
                        ImGui::CloseCurrentPopup();
                    ImGui::EndPopup();
                }

                ImGui::Dummy(ImVec2(0.0f, 10.0f));
                ImGui::TreePop();
            }
        }
    }

    ImGui::Separator();
}

// TODO @WIP:
void EditorGui::progressPlotWidget(bool animate=true)
{
    static float progress = 0.0f, progress_dir = 1.0f;
    if (animate)
    {
        progress += progress_dir * 0.4f * ImGui::GetIO().DeltaTime;
        if (progress >= +1.1f) { progress = +1.1f; progress_dir *= -1.0f; }
        if (progress <= -0.1f) { progress = -0.1f; progress_dir *= -1.0f; }
    }

    // Typically we would use ImVec2(-1.0f,0.0f) or ImVec2(-FLT_MIN,0.0f) to use all available width,
    // or ImVec2(width,0.0f) for a specified width. ImVec2(0.0f,0.0f) uses ItemWidth.
    ImGui::ProgressBar(progress, ImVec2(0.0f, 0.0f));
    ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
    //ImGui::Text("Test");
}
