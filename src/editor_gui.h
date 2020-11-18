#pragma once

#include "IMGUI/imgui.h"
#include "IMGUI/imgui_impl_glfw.h"
#include "IMGUI/imgui_impl_opengl3.h"

#include "FONTAWESOME/IconsFontAwesome5.h"


struct EditorGui
{
    EditorGui(Window *window);
    ~EditorGui();
    void newFrame();
    void draw();

    void performanceInfoOverlay(Window *window, Renderer *renderer, PlateformInfo *info);
    void camera_settings_collapse_header(Camera *camera);
    void window_settings_collapse_header(Window *window, InputState *input);
    void terrain_settings_collapse_header(Terrain *terrain,
                                          uint32 &resolution,
                                          uint32 maxResolution,
                                          bool *showSkybox);
    void entities_list_collapse_header(std::map<uint32, Entity*> *Scene,
                                                  uint32 *selectedEntity,
                                                  float32 pickingSphereRadius);
    void bigAndDirtyPanel(Window *window,
                          InputState *input,
                          Camera *camera,
                          Terrain *terrain,
                          uint32 &terrainResolution,
                          uint32 terrainMaxResolution,
                          bool *showSkybox,
                          std::map<uint32, Entity*> *entities,
                          uint32 *selectedEntity,
                          float32 pickingSphereRadius,
                          bool &focus);

    
    bool activeWindow;
    const float32 f32_zero = 0.1f, f32_two = 2.0f, f32_ten = 10.0f, f32_360 = 360.0f;

#if 0
    OPENFILENAME g_Ofn;
    char g_szFile[260];
    HWND g_hwnd;
#endif

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
    io.Fonts->AddFontFromFileTTF( "assets/fonts/webfonts/fa-solid-900.ttf", 16.0f, &icons_config, icons_ranges );

    this->activeWindow = false;

#if 0
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
#endif
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
void EditorGui::performanceInfoOverlay(Window *window,
                                       Renderer *renderer,
                                       PlateformInfo *info)
{
    ImGui::SetNextWindowPos(ImVec2((float32)(window->getWidth() - 210), 10));
    ImGui::SetNextWindowBgAlpha(0.35f);
    if (ImGui::Begin("window_stats", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav))
    {
        ImGui::Text(info->vendor);
        ImGui::Text(info->graphicAPI);
        ImGui::Text(info->versionAPI);
        ImGui::Separator();
        ImGui::Text("ms/f: %.3fms", window->time->deltaTime);
        //ImGui::Text("fps: %d", Window->time->FPS);
        //ImGui::Text("mcy/f: %d", Window->time->megaCyclePerFrame);
        ImGui::Text("drawCalls: %d", renderer->stats.drawCalls);
        ImGui::End();
    }
}

// TODO: split in dedicated widget
void EditorGui::bigAndDirtyPanel(Window *window,
                     InputState *input,
                     Camera *camera,
                     Terrain *terrain,
                     uint32 &terrainResolution,
                     uint32 terrainMaxResolution,
                     bool *showSkybox,
                     std::map<uint32, Entity*> *entities,
                     uint32 *selectedEntity,
                     float32 pickingSphereRadius,
                     bool &focus)
{
    ImGui::SetNextWindowPos(ImVec2(10, 10));
    ImGui::SetNextWindowSize(ImVec2(410, (float32)(window->getHeight() - 20)));
    ImGui::Begin("settings", nullptr, ImGuiWindowFlags_NoResize);

    this->window_settings_collapse_header(window, input);
    this->camera_settings_collapse_header(camera);
    this->terrain_settings_collapse_header(terrain, terrainResolution, terrainMaxResolution, showSkybox);
    this->entities_list_collapse_header(entities, selectedEntity, pickingSphereRadius);
    
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow))
        focus = true;
    else
        focus = false;
    ImGui::End();
}

void EditorGui::window_settings_collapse_header(Window *window, InputState *input)
{
    if (ImGui::CollapsingHeader("Window settings", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImVec2 bSize(40, 20);
        ImGui::Text("SCREEN: %d x %d", window->getWidth(), window->getHeight());
        ImGui::Text("MousePos: %d x %d",
                    (uint32)input->mouse->posX,
                    (uint32)input->mouse->posY);
        ImGui::Separator();

        ImGui::PushID(1);
        if (ImGui::Button(window->getVsync() ? "on" : "off", bSize))
            window->toggleVsync();
        ImGui::SameLine();
        ImGui::Text("VSYNC: ");
        ImGui::PopID();

        ImGui::PushID(2);
        if (ImGui::Button(window->debug ? "on" : "off", bSize))
            window->debug = !window->debug;
        ImGui::SameLine();
        ImGui::Text("DEBUG: ");
        ImGui::PopID();
    }
}

void EditorGui::terrain_settings_collapse_header(Terrain *terrain,
                                                 uint32 &resolution,
                                                 uint32 maxResolution,
                                                 bool *showSkybox)
{
    if (ImGui::CollapsingHeader("terrain", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Dummy(ImVec2(0.0f, 3.0f));	
        ImGui::Text( ICON_FA_CUBE );
        ImGui::SameLine();
        ImGui::Text("%s", terrain->entity->model->objFilename.c_str());
        ImGui::Text("path: %s", terrain->entity->model->directory.c_str());

        if (ImGui::Button("change"))
        {

#if 0
            if (GetOpenFileName(&g_Ofn)==TRUE)
            {
                model_t *loadedModel = LoadModelFromFile(g_szFile);
                if (loadedModel != nullptr)
                {
                    Delete(terrain->entity->model);
                    terrain->entity->model = loadedModel;
                    terrain->IsGenerated = false;
                }
            }
#endif

        }

        //ImGui::SliderInt("res", &(int)resolution, 0, maxResolution);
        ImGui::SameLine();
        ImGui::Text("max: %dx%d", maxResolution, maxResolution);
        ImGui::Separator();
        ImGui::Checkbox("skybox", showSkybox);
        ImGui::Separator();
    }
}

void EditorGui::entities_list_collapse_header(std::map<uint32, Entity*> *Scene,
                                              uint32 *selectedEntity,
                                              float32 pickingSphereRadius)
{
    if (ImGui::CollapsingHeader("Entity list", ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (ImGui::Button("add"))
        {
#if 0
            if (GetOpenFileName(&g_Ofn)==TRUE)
            {
                uint32 id = (uint32)Scene->size() + 1;
                model_t *loadedModel = LoadModelFromFile(g_szFile);
                Entity *entity = new Entity;
                entity->model = loadedModel;
                entity->PickingSphere =  CreatePrimitiveSphereMesh(0.0f, pickingSphereRadius, 15, 15);

                Scene->insert({id, entity});
            }
#endif
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
        {

#if 0
            if (GetOpenFileName(&g_Ofn)==TRUE)
            {
                OpenSceneFromTextFormat(g_szFile, Scene);
            }
#endif
            
        }
        ImGui::SameLine();

        if (ImGui::Button("save"))
        {

#if 0
            if (GetSaveFileName(&g_Ofn)==TRUE)
            {
                SaveSceneInTextFormat(g_szFile, Scene);
            }
#endif
        }
        ImGui::Separator();
    }
}

void EditorGui::camera_settings_collapse_header(Camera *camera)
{
    if (ImGui::CollapsingHeader("camera settings", ImGuiTreeNodeFlags_DefaultOpen))
    {
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

        ImVec2 bSize(100, 20);
        if (ImGui::Button("Reset Default", bSize))
        {
            camera->position = glm::vec3(0.0f, 5.0f, 10.0f);
            camera->settings->yaw = -90.0f;
            camera->settings->pitch = 0.0f;
            camera->settings->fov = 45.0f;
            camera->processMovementAngles(0.0f, 0.0f);
        }
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
