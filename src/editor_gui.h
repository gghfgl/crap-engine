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

    void performanceInfoOverlay(Renderer *renderer, PlateformInfo *info);

    void makePanel(float32 posX, float32 posY);
    void endPanel();

    void windowAndInputSettings(InputState *input);
    void cameraSettings(Camera *camera);
    void groundSettings(int32 &resolution,
                        uint32 maxResolution,
                        uint32 &currentGroundIndex,
                        std::map<uint32, Ground*> *Grounds);

    void environmentSettings(Ground *ground,
                             int32 &resolution,
                             uint32 maxResolution,
                             bool *showSkybox);
    void entitiesSettings(std::map<uint32, Entity*> *Scene,
                          uint32 *selectedEntity,
                          float32 pickingSphereRadius);
    
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
                                       PlateformInfo *info)
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

void EditorGui::groundSettings(int32 &resolution,
                               uint32 maxResolution,
                               uint32 &currentGroundIndex,
                               std::map<uint32, Ground*> *Grounds)
{
    if (ImGui::CollapsingHeader("ground", ImGuiTreeNodeFlags_DefaultOpen))
    {
        // Resolution slider
        ImGui::Dummy(ImVec2(0.0f, 3.0f));
        ImGui::PushItemWidth(-FLT_MIN);
        ImGui::Text("resolution (max %dx%d):", maxResolution, maxResolution);
        ImGui::SliderInt("", &resolution, 0, maxResolution);
        ImGui::Separator();

        // Add ground button
        ImGui::Dummy(ImVec2(0.0f, 3.0f));	
        if (ImGui::Button("add"))
            ImGui::OpenPopup("AddGround");                    

        // Add ground modal
        if (ImGui::BeginPopupModal("AddGround"))
        {
            ImGui::Dummy(ImVec2(0.0f, 3.0f));	
            ImGui::Text("add a new ground to the list:");
            static char str0[32] = "give me a name";
            ImGui::PushItemWidth(-FLT_MIN);
            ImGui::InputText("", str0, IM_ARRAYSIZE(str0));
            ImGui::Separator();

            ImGui::Dummy(ImVec2(0.0f, 3.0f));
            if (ImGui::Button("Add", ImVec2(120, 0)))
            {
                char *name = new char[32];
                strncpy(name, str0, 32);
                name[32 - 1] = '\0';
                
                Ground *ground = new Ground(name, resolution, "");
                std::time_t timestamp = std::time(nullptr);
                Grounds->insert({static_cast<uint32>(timestamp), ground});

                // Reset input placeholder
                strncpy(str0, "give me a name", 32);
                
                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();
            if (ImGui::Button("Close", ImVec2(120, 0)))
                ImGui::CloseCurrentPopup();

            ImGui::EndPopup();
        }

        // Save ground list into file
        ImGui::SameLine();
        if (ImGui::Button("save"))
        {
            std::time_t timestamp = std::time(nullptr);
            igfd::ImGuiFileDialog::Instance()->OpenDialog("SaveGroundListInTextFormat", "Choose File", ".list", ".", "crap_grounds_"+std::to_string(timestamp));
        }

        if (igfd::ImGuiFileDialog::Instance()->FileDialog("SaveGroundListInTextFormat", ImGuiWindowFlags_NoCollapse, this->dialogMinSize, this->dialogMaxSize))
        {
            if (igfd::ImGuiFileDialog::Instance()->IsOk == true)
            {
                std::string filePathName = igfd::ImGuiFileDialog::Instance()->GetFilePathName();
                SaveGroundListInTextFormat(filePathName.c_str(), Grounds);
            }

            igfd::ImGuiFileDialog::Instance()->CloseDialog("SaveGroundListInTextFormat");
        }

        // Open ground list fom file
        ImGui::SameLine();
        if (ImGui::Button("open"))
            igfd::ImGuiFileDialog::Instance()->OpenDialog("OpenGroundListFromFile", "Choose File", ".list", ".");

        if (igfd::ImGuiFileDialog::Instance()->FileDialog("OpenGroundListFromFile", ImGuiWindowFlags_NoCollapse, this->dialogMinSize, this->dialogMaxSize))
        {
            if (igfd::ImGuiFileDialog::Instance()->IsOk == true)
            {
                std::string filePathName = igfd::ImGuiFileDialog::Instance()->GetFilePathName();
                OpenGroundListFromFile(filePathName.c_str(), Grounds);
            }

            igfd::ImGuiFileDialog::Instance()->CloseDialog("OpenGroundListFromFile");
        }
        
        ImGui::Dummy(ImVec2(0.0f, 3.0f));	

        // Ground list
        static uint32 selectedIndex = 0;
        for (auto it = Grounds->begin(); it != Grounds->end(); it++)	 
        {
            if (ImGui::TreeNode((void*)(intptr_t)it->first, "%s %s", it->second->name, (currentGroundIndex == it->first) ? "[selected]" : ""))
            {
                // Ground data
                ImGui::Dummy(ImVec2(0.0f, 3.0f));	
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

                // Load ground model button
                if (ImGui::SmallButton("load"))
                {
                    selectedIndex = it->first;
                    igfd::ImGuiFileDialog::Instance()->OpenDialog("LoadGroundModel", "Choose File", ".obj", ".");
                }

                // Delete button
                ImGui::SameLine();
                if (ImGui::SmallButton("rename"))
                {
                    ImGui::OpenPopup("RenameGround");                    
                }

                // Select ground button
                ImGui::SameLine();
                if (ImGui::SmallButton("select"))
                    currentGroundIndex = it->first;

                // Delete button
                ImGui::SameLine();
                if (ImGui::SmallButton("delete"))
                {
                    selectedIndex = it->first;
                    ImGui::OpenPopup("DeleteGround");                    
                }

                // Rename ground
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

                        delete it->second->name;
                        it->second->name = rename;

                        // Reset input placeholder
                        strncpy(str1, "give me a name", 32);

                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::EndPopup();
                }
                
                // Delete ground modal
                if (ImGui::BeginPopupModal("DeleteGround"))
                {
                    ImGui::Text("selected ground will be deleted.\nThis operation cannot be undone!\n\n");
                    ImGui::Separator();

                    if (ImGui::Button("OK", ImVec2(120, 0))) {
                        delete it->second;
                        Grounds->erase(it->first);

                        if (it->first == currentGroundIndex)
                            currentGroundIndex = 0;

                        ImGui::CloseCurrentPopup();
                    }

                    selectedIndex = 0;
                    ImGui::SetItemDefaultFocus();
                    ImGui::SameLine();
                    if (ImGui::Button("Cancel", ImVec2(120, 0)))
                        ImGui::CloseCurrentPopup();
                    ImGui::EndPopup();
                }

                ImGui::TreePop();
            }
        }

        // Open dialog load ground model file
        if (igfd::ImGuiFileDialog::Instance()->FileDialog("LoadGroundModel", ImGuiWindowFlags_NoCollapse, this->dialogMinSize, this->dialogMaxSize))
        {
            if (igfd::ImGuiFileDialog::Instance()->IsOk == true)
            {
                std::string filePathName = igfd::ImGuiFileDialog::Instance()->GetFilePathName();
                std::string filePath = igfd::ImGuiFileDialog::Instance()->GetCurrentPath();

                Model *loadedModel = new Model(filePathName);
                if (loadedModel != nullptr)
                {
                    delete (*Grounds)[selectedIndex]->entity->model;
                    (*Grounds)[selectedIndex]->entity->model = loadedModel;
                    (*Grounds)[selectedIndex]->isGenerated = false;
                }
            }

            selectedIndex = 0;
            igfd::ImGuiFileDialog::Instance()->CloseDialog("LoadGroundModel");
        }

    }
}

void EditorGui::environmentSettings(Ground *ground,
                                    int32 &resolution,
                                    uint32 maxResolution,
                                    bool *showSkybox)
{
    if (ImGui::CollapsingHeader("environment", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Dummy(ImVec2(0.0f, 3.0f));	
        ImGui::Text( ICON_FA_CUBE );
        ImGui::SameLine();
        ImGui::Text("%s", ground->entity->model->objFilename.c_str());
        ImGui::Text("path: %s", ground->entity->model->directory.c_str());

        if (ImGui::Button("change"))
            igfd::ImGuiFileDialog::Instance()->OpenDialog("SelectEnvModel", "Choose File", ".obj", ".");

        if (igfd::ImGuiFileDialog::Instance()->FileDialog("SelectEnvModel", ImGuiWindowFlags_NoCollapse, this->dialogMinSize, this->dialogMaxSize))
        {
            if (igfd::ImGuiFileDialog::Instance()->IsOk == true)
            {
                std::string filePathName = igfd::ImGuiFileDialog::Instance()->GetFilePathName();

                Model *loadedModel = new Model(filePathName);
                if (loadedModel != nullptr)
                {
                    delete ground->entity->model;
                    ground->entity->model = loadedModel;
                    ground->isGenerated = false;
                }
            }

            igfd::ImGuiFileDialog::Instance()->CloseDialog("SelectEnvModel");
        }

        ImGui::SliderInt("res", &resolution, 0, maxResolution);
        ImGui::SameLine();
        ImGui::Text("max: %dx%d", maxResolution, maxResolution);
        ImGui::Separator();
        ImGui::Checkbox("skybox", showSkybox);
        ImGui::Separator();
    }
}

void EditorGui::entitiesSettings(std::map<uint32, Entity*> *Scene,
                                 uint32 *selectedEntity,
                                 float32 pickingSphereRadius)
{
    if (ImGui::CollapsingHeader("Entity list", ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (ImGui::Button("add"))
            igfd::ImGuiFileDialog::Instance()->OpenDialog("SelectEntityModel", "Choose File", ".obj", ".");

        if (igfd::ImGuiFileDialog::Instance()->FileDialog("SelectEntityModel", ImGuiWindowFlags_NoCollapse, this->dialogMinSize, this->dialogMaxSize))
        {
            if (igfd::ImGuiFileDialog::Instance()->IsOk == true)
            {
                std::string filePathName = igfd::ImGuiFileDialog::Instance()->GetFilePathName();
                //std::string filePath = igfd::ImGuiFileDialog::Instance()->GetCurrentPath();

                uint32 id = (uint32)Scene->size() + 1;
                Entity *entity = new Entity;
                entity->model = new Model(filePathName);;
                entity->pickingSphere = new Mesh(0.0f, pickingSphereRadius, 15, 15);

                Scene->insert({id, entity});
            }

            igfd::ImGuiFileDialog::Instance()->CloseDialog("SelectEntityModel");
        }
        ImGui::Separator();

        ImGui::BeginChild("left pane", ImVec2(120, 150));
        ImGui::Dummy(ImVec2(0.0f, 3.0f));	
        for (auto it = Scene->begin(); it != Scene->end(); it++)	 
        {
            ImGui::Text( ICON_FA_CUBE );
            ImGui::SameLine();
            char label[128];
            sprintf(label, "%d_%s", it->first, it->second->model->objFilename.c_str());
            if (ImGui::Selectable(label, *selectedEntity == it->first))
                *selectedEntity = it->first;
        }

        ImGui::EndChild();
        ImGui::SameLine();

        ImGui::BeginChild("right pane", ImVec2(0, 150));
        if (*selectedEntity != 0)
        {
            ImGui::Text("mem: %p", &(*Scene)[*selectedEntity]);
            ImGui::Text("ID: %03d", *selectedEntity);
            ImGui::Text("Label: %s", (*Scene)[*selectedEntity]->model->objFilename.c_str());
            ImGui::Text("Filepath: %s", (*Scene)[*selectedEntity]->model->directory.c_str());
            ImGui::Text("Pos x=%.2f y=%.2f z=%.2f",
                        (*Scene)[*selectedEntity]->position.x,
                        (*Scene)[*selectedEntity]->position.y,
                        (*Scene)[*selectedEntity]->position.z);
            ImGui::SliderScalar("scale", ImGuiDataType_Float,
                                &(*Scene)[*selectedEntity]->scale,
                                &f32_zero, &f32_ten);
            ImGui::SliderScalar("rotate", ImGuiDataType_Float,
                                &(*Scene)[*selectedEntity]->rotate,
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
                delete (*Scene)[*selectedEntity];
                (*Scene).erase(*selectedEntity);
                *selectedEntity = 0;
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
            igfd::ImGuiFileDialog::Instance()->OpenDialog("OpenEntityListFromFileTextFormat", "Choose File", ".list", ".");

        if (igfd::ImGuiFileDialog::Instance()->FileDialog("OpenEntityListFromFileTextFormat", ImGuiWindowFlags_NoCollapse, this->dialogMinSize, this->dialogMaxSize))
        {
            if (igfd::ImGuiFileDialog::Instance()->IsOk == true)
            {
                std::string filePathName = igfd::ImGuiFileDialog::Instance()->GetFilePathName();
                //std::string filePath = igfd::ImGuiFileDialog::Instance()->GetCurrentPath();
                OpenEntityListFromFileTextFormat(filePathName.c_str(), Scene);
            }

            igfd::ImGuiFileDialog::Instance()->CloseDialog("OpenEntityListFromFileTextFormat");
        }
        ImGui::SameLine();

        if (ImGui::Button("save"))
        {
            
            std::time_t timestamp = std::time(nullptr);
            igfd::ImGuiFileDialog::Instance()->OpenDialog("SaveEntityListInTextFormat", "Choose File", ".list", ".", "crap"+std::to_string(timestamp));
        }

        if (igfd::ImGuiFileDialog::Instance()->FileDialog("SaveEntityListInTextFormat", ImGuiWindowFlags_NoCollapse, this->dialogMinSize, this->dialogMaxSize))
        {
            if (igfd::ImGuiFileDialog::Instance()->IsOk == true)
            {
                std::string filePathName = igfd::ImGuiFileDialog::Instance()->GetFilePathName();
                //std::string filePath = igfd::ImGuiFileDialog::Instance()->GetCurrentPath();
                SaveEntityListInTextFormat(filePathName.c_str(), Scene);
            }

            igfd::ImGuiFileDialog::Instance()->CloseDialog("SaveEntityListInTextFormat");
        }

        ImGui::Separator();
    }
}
