#pragma once

#include "IMGUI/imgui.h"
#include "IMGUI/imgui_impl_glfw.h"
#include "IMGUI/imgui_impl_opengl3.h"

#include "FONTAWESOME/IconsFontAwesome5.h"

static void window_settings_collapse_header(WindowWrapper *Window, InputState *Input);
static void terrain_settings_collapse_header(terrain_t *Terrain, uint32 &resolution, uint32 maxResolution, bool *showSkybox);
static void entities_list_collapse_header(std::map<uint32, entity_t*> *entities, uint32 *selectedEntity, float32 pickingSphereRadius);
static void camera_settings_collapse_header(Camera *Camera);

static bool g_ActiveWindow = false;
const float32 f32_zero = 0.1f, f32_two = 2.0f, f32_ten = 10.0f, f32_360 = 360.0f; // TODO: static

#if 0
OPENFILENAME g_Ofn;
char g_szFile[260];
HWND g_hwnd;
#endif

void InitEditorGui(WindowWrapper* Window)
{
    const char* glsl_version = "#version 450";
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    //ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(Window->Context, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Fonts
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontDefault();
    static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
    ImFontConfig icons_config; icons_config.MergeMode = true; icons_config.PixelSnapH = true;
    //io.Fonts->AddFontFromFileTTF( "../assets/fonts/Font-Awesome-master/Font-Awesome-master/webfonts/fa-regular-400.ttf", 16.0f, &icons_config, icons_ranges );
    io.Fonts->AddFontFromFileTTF( "assets/fonts/webfonts/fa-solid-900.ttf", 16.0f, &icons_config, icons_ranges );

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

void DeleteEditorGui()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void NewFrameEditorGui()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void RenderEditorGui()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

// ================================================================

// TODO: switch to string rendering
void ShowWindowStatsOverlay(WindowWrapper *Window, renderer_t *Renderer)
{
    ImGui::SetNextWindowPos(ImVec2((float32)(Window->getWidth() - 210), 10));
    ImGui::SetNextWindowBgAlpha(0.35f);
    if (ImGui::Begin("window_stats", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav))
    {
        // TODO: string rendering
        /* ImGui::Text(Window->RenderAPIinfo.Vendor); */
        /* ImGui::Text(Window->RenderAPIinfo.Renderer); */
        /* ImGui::Text(Window->RenderAPIinfo.Version); */
        /* ImGui::Separator(); */
        ImGui::Text("ms/f: %.3fms", Window->Time->deltaTime);
        //ImGui::Text("fps: %d", Window->Time->FPS);
        //ImGui::Text("mcy/f: %d", Window->Time->megaCyclePerFrame);
        ImGui::Text("drawCalls: %d", Renderer->Stats.DrawCalls);
        ImGui::End();
    }
}

// TODO: pass func signature directly? panel as a service?
void ShowEditorPanel(WindowWrapper *Window,
                     InputState *Input,
                     Camera *Camera,
                     terrain_t *Terrain,
                     uint32 &terrainResolution,
                     uint32 terrainMaxResolution,
                     bool *showSkybox,
                     std::map<uint32, entity_t*> *entities,
                     uint32 *selectedEntity,
                     float32 pickingSphereRadius,
                     bool &focus)
{
    ImGui::SetNextWindowPos(ImVec2(10, 10));
    ImGui::SetNextWindowSize(ImVec2(410, (float32)(Window->getHeight() - 20)));
    ImGui::Begin("settings", nullptr, ImGuiWindowFlags_NoResize);

    window_settings_collapse_header(Window, Input);
    camera_settings_collapse_header(Camera);
    terrain_settings_collapse_header(Terrain, terrainResolution, terrainMaxResolution, showSkybox);
    entities_list_collapse_header(entities, selectedEntity, pickingSphereRadius);
    
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow))
        focus = true;
    else
        focus = false;
    ImGui::End();
}

static void window_settings_collapse_header(WindowWrapper *Window, InputState *Input)
{
    if (ImGui::CollapsingHeader("Window settings", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImVec2 bSize(40, 20);
        ImGui::Text("SCREEN: %d x %d", Window->getWidth(), Window->getHeight());
        ImGui::Text("MousePos: %d x %d",
                    (uint32)Input->Mouse->posX,
                    (uint32)Input->Mouse->posY);
        ImGui::Separator();

        ImGui::PushID(1);
        if (ImGui::Button(Window->getVsync() ? "on" : "off", bSize))
            Window->toggleVsync();
        ImGui::SameLine();
        ImGui::Text("VSYNC: ");
        ImGui::PopID();

        ImGui::PushID(2);
        if (ImGui::Button(Window->debug ? "on" : "off", bSize))
            Window->debug = !Window->debug;
        ImGui::SameLine();
        ImGui::Text("DEBUG: ");
        ImGui::PopID();
    }
}

// TODO: 
static void terrain_settings_collapse_header(terrain_t *Terrain,
                                             uint32 &resolution,
                                             uint32 maxResolution,
                                             bool *showSkybox)
{
    if (ImGui::CollapsingHeader("Terrain", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Dummy(ImVec2(0.0f, 3.0f));	
        ImGui::Text( ICON_FA_CUBE );
        ImGui::SameLine();
        ImGui::Text("%s", Terrain->Entity->Model->ObjFilename.c_str());
        ImGui::Text("path: %s", Terrain->Entity->Model->Directory.c_str());

        if (ImGui::Button("change"))
        {

#if 0
            if (GetOpenFileName(&g_Ofn)==TRUE)
            {
                model_t *loadedModel = LoadModelFromFile(g_szFile);
                if (loadedModel != nullptr)
                {
                    Delete(Terrain->Entity->Model);
                    Terrain->Entity->Model = loadedModel;
                    Terrain->IsGenerated = false;
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

static void entities_list_collapse_header(std::map<uint32, entity_t*> *Scene,
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
                entity_t *entity = new entity_t;
                entity->Model = loadedModel;
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
            sprintf(label, "%d_%s", it->first, it->second->Model->ObjFilename.c_str());
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
            ImGui::Text("Label: %s", (*Scene)[*selectedEntity]->Model->ObjFilename.c_str());
            ImGui::Text("Filepath: %s", (*Scene)[*selectedEntity]->Model->Directory.c_str());
            ImGui::Text("Pos x=%.2f y=%.2f z=%.2f",
                        (*Scene)[*selectedEntity]->Position.x,
                        (*Scene)[*selectedEntity]->Position.y,
                        (*Scene)[*selectedEntity]->Position.z);
            ImGui::SliderScalar("scale", ImGuiDataType_Float,
                                &(*Scene)[*selectedEntity]->Scale,
                                &f32_zero, &f32_ten);
            ImGui::SliderScalar("rotate", ImGuiDataType_Float,
                                &(*Scene)[*selectedEntity]->Rotate,
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
                Delete((*Scene)[*selectedEntity]);
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

static void camera_settings_collapse_header(Camera *Camera)
{
    if (ImGui::CollapsingHeader("Camera settings", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Text("yaw: %.2f", Camera->Settings->yaw);
        ImGui::Text("pitch: %.2f", Camera->Settings->pitch);
        ImGui::Text("speed: %.2f", Camera->Settings->speed);
        ImGui::Text("sensitivity: %.2f", Camera->Settings->sensitivity);
        ImGui::Text("fov: %.2f", Camera->Settings->fov);
        ImGui::Text("pos: %.2f, %.2f, %.2f",
                    Camera->position.x,
                    Camera->position.y,
                    Camera->position.z);
        ImGui::Text("worldup: %.2f, %.2f, %.2f",
                    Camera->worldUp.x,
                    Camera->worldUp.y,
                    Camera->worldUp.z);

        ImVec2 bSize(100, 20);
        if (ImGui::Button("Reset Default", bSize))
        {
            Camera->position = glm::vec3(0.0f, 5.0f, 10.0f);
            Camera->Settings->yaw = -90.0f;
            Camera->Settings->pitch = 0.0f;
            Camera->Settings->fov = 45.0f;
            Camera->processMovementAngles(0.0f, 0.0f);
        }
        ImGui::SameLine();
        if (ImGui::Button("Reset Up", bSize))
        {
            Camera->position = glm::vec3(0.0f, 30.0f, 0.0f);
            Camera->Settings->yaw = -90.0f;
            Camera->Settings->pitch = -90.0f;
            Camera->Settings->fov = 45.0f;
            Camera->processMovementAngles(0.0f, 0.0f);
        }
        ImGui::Separator();
    }
}
