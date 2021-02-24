#include "global.h"
#include "game.h"
#include "game_gui.h"

void RunGame(Window *Window, InputState *Input, PlateformInfo *Info, GlobalState *GlobalState)
{
    // Init game state
    GameState gs;
    
    // Init camera
    Camera *camera = new Camera(
        glm::vec3(0.0f, 30.0f, 20.0f), // position
        glm::vec3(0.0f, 0.0f, 0.0f),   // lookAt
        glm::vec3(0.0f, 1.0f, 0.0f),   // worldUp
        45.0f,                         // fov
        -60.0f,                        // pitch
        (float32)Window->GetWidth() / (float32)Window->GetHeight(), // aspectRatio
        gs.nearPlane, gs.farPlane);    // near plane & far plane

    // Init renderer
    Renderer *renderer = new Renderer();

    // Init GUI
    GameGui gui = GameGui(Window);
    
    // Compile and cache shaders
    ShaderCache *sCache = new ShaderCache();
    int32 error = sCache->CompileShadersFromDirectory("./shaders", camera->projectionMatrix);
    if (error != 0)
    {
        Log::error("EXITCODE:111 Failed to compile shaders");
        exit(111);
    }

    // =================================================

    // ReferenceGrid
    std::vector<uint32> uEmpty; // TODO: init default in constructor?
    std::vector<Texture> tEmpty;
    std::vector<Vertex> vReferenceGrid(g_ReferenceGridResolution * 4 + 4, Vertex());
    Mesh *ReferenceGrid = new Mesh(vReferenceGrid, uEmpty, tEmpty);
    renderer->PrepareReferenceGridSubData(ReferenceGrid, g_ReferenceGridResolution);

    // Aim ray
    std::vector<Vertex> vRay(2, Vertex());
    Mesh *AimPlayerRay = new Mesh(vRay, uEmpty, tEmpty);

    // Test Payer
    Player *testPlayer = new Player("testPlayer", "./assets/models/untitled-scene-obj/untitled.obj", glm::vec3(0.0f));

    // Modules environment
    std::map<uint32, Module*> *Modules = new std::map<uint32, Module*>;
    error = OpenModuleListFromFile("./assets/lists/crap_modules_1614124982.list", Modules);
    if (error != 0)
    {
        Log::error("\terror while opening file: %s", "./assets/lists/crap_modules_1614124982.list");
        GlobalState->currentMode = EXIT_MODE;
        return;
    }

    // =================================================

    while (GlobalState->currentMode == GAME_MODE)
    {
        Window->UpdateTime();
        Window->PollEvents();

        /********************************************************
         *                                                      *
         *        NOTE: I/O Keyboard and Mouse                  *
         *                                                      *
         ********************************************************/

        if (Input->keyboard->isPressed[keyboard::CRAP_KEY_ESCAPE])
            GlobalState->currentMode = EXIT_MODE;

        if (Input->keyboard->isPressed[keyboard::CRAP_KEY_E])
            GlobalState->currentMode = EDITOR_MODE;

        if (Input->keyboard->isPressed[keyboard::CRAP_KEY_W])
            testPlayer->entity->UpdatePositionFromDirection(ENTITY_FORWARD, Window->time->deltaTime);
        if (Input->keyboard->isPressed[keyboard::CRAP_KEY_S])
            testPlayer->entity->UpdatePositionFromDirection(ENTITY_BACKWARD, Window->time->deltaTime);
        if (Input->keyboard->isPressed[keyboard::CRAP_KEY_A])
            testPlayer->entity->UpdatePositionFromDirection(ENTITY_LEFT, Window->time->deltaTime);
        if (Input->keyboard->isPressed[keyboard::CRAP_KEY_D])
            testPlayer->entity->UpdatePositionFromDirection(ENTITY_RIGHT, Window->time->deltaTime);

        if (Input->mouse->scrollOffsetY != 0.0f)
        {
            if (Input->GetMouseScrollOffsetY() > 0)
                camera->UpdatePositionFromDirection(CAMERA_FORWARD, Window->time->deltaTime, 30.0f);
            else
                camera->UpdatePositionFromDirection(CAMERA_BACKWARD, Window->time->deltaTime, 30.0f);
        }

        // if (Input->mouse->leftButton)
        // {
        //     Input->UpdateMouseOffsets();
        //     camera->UpdateArcballFromAngle(Input->mouse->offsetX, Input->mouse->offsetY);
        // }

        /********************************************************
         *                                                      *
         *                    NOTE: Simulate                    *
         *                                                      *
         ********************************************************/

        // Mouse ray
        glm::vec3 mouseRayWorld = MouseRayDirectionWorld((float32)Input->mouse->posX,
                                                    (float32)Input->mouse->posY,
                                                    Window->GetWidth(),
                                                    Window->GetHeight(),
                                                    camera->projectionMatrix,
                                                    camera->viewMatrix);

        // Mouse ray intersec with plane
        glm::vec3 mouseRayIntersection = glm::vec3(0.0f);
        if (!RayPlaneIntersection(camera->position,
                                  mouseRayWorld, glm::vec3(0.0f),
                                  glm::vec3(0.0f, 1.0f, 0.0f),
                                  &mouseRayIntersection))
            mouseRayIntersection = glm::vec3(0.0f);

        // Camera follow player
        glm::vec3 behindPlayer = glm::vec3(testPlayer->entity->position.x, camera->position.y, testPlayer->entity->position.z + 20.0f);
        camera->SetCameraView(behindPlayer, testPlayer->entity->position, camera->worldUp);

        // Player follow mouse pointer
        testPlayer->entity->UpdateRotationFollowVec(mouseRayIntersection, glm::vec2(Input->mouse->posX, Input->mouse->posY), gs.farPlane);

        /********************************************************
         *                                                      *
         *                 NOTE: Rendering                      *
         *                                                      *
         ********************************************************/

        // Clear renderer
        renderer->ResetStats();
        renderer->NewContext();
        glm::mat4 viewMatrix = camera->viewMatrix;

        // Draw ReferenceGrid
        Shader *colorShader = sCache->GetShader("color");
        colorShader->UseProgram();
        colorShader->SetUniform4fv("view", viewMatrix);
        colorShader->SetUniform4fv("model", glm::mat4(1.0f));

        colorShader->SetUniform4f("color", glm::vec4(0.360f, 1.0f, 0.360f, 1.0f));
        renderer->DrawLines(ReferenceGrid, 1.0f);
        
        // Draw AimPlayerRay
        renderer->PrepareRaySubData(AimPlayerRay, testPlayer->entity->position, mouseRayIntersection);
        colorShader->SetUniform4f("color", glm::vec4(1.0f, 0.8f, 0.0f, 1.0));
        renderer->DrawLines(AimPlayerRay, 1.0f);

        // Draw Player
        Shader *defaultShader = sCache->GetShader("default");
        defaultShader->UseProgram();
        defaultShader->SetUniform4fv("view", viewMatrix);
        defaultShader->SetUniform4fv("model", glm::mat4(1.0f));
        
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(testPlayer->entity->position.x,
                                                testPlayer->entity->position.y + float32(testPlayer->entity->scale) / 2,
                                                testPlayer->entity->position.z));
        model = glm::scale(model, glm::vec3(testPlayer->entity->scale));
        model = glm::rotate(model, glm::radians(testPlayer->entity->rotate), glm::vec3(0.0f, 1.0f, 0.0f));
        defaultShader->SetUniform4fv("model", model);
        renderer->DrawModel(testPlayer->entity->model, defaultShader);

        // Player BoundingBox
        if (gs.showBoundingBox)
        {
            colorShader->UseProgram();
            colorShader->SetUniform4fv("view", viewMatrix);
            colorShader->SetUniform4fv("model", model);
            colorShader->SetUniform4f("color", glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
            renderer->DrawBoundingBox(testPlayer->entity->model->boundingBox);
        }
        
        // TODO: not modules anymore
        // Draw Modules
        for (auto it = Modules->begin(); it != Modules->cend(); it++)
        {
            if (it->second->entity->model != nullptr)
            {
                Shader *defaultShader = sCache->GetShader("default");
                defaultShader->UseProgram();
                defaultShader->SetUniform4fv("view", viewMatrix);
                defaultShader->SetUniform4fv("model", glm::mat4(1.0f));

                // bool isSelected = false;
                // if (es.hoveredModule == it->first || es.selectedModuleIndex == it->first)
                //     isSelected = true;

                // if (es.dragModule == it->first)
                //     it->second->entity->position = pIntersection;

                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(it->second->entity->position.x,
                                                        it->second->entity->position.y + float32(it->second->entity->scale) / 2,
                                                        it->second->entity->position.z));
                model = glm::scale(model, glm::vec3(it->second->entity->scale));
                model = glm::rotate(model, glm::radians(it->second->entity->rotate), glm::vec3(0.0f, 1.0f, 0.0f));
                defaultShader->SetUniform4fv("model", model);
                renderer->DrawModel(it->second->entity->model, defaultShader);

                // Module BoundingBox
                if (gs.showBoundingBox)
                {
                    colorShader->UseProgram();
                    colorShader->SetUniform4fv("view", viewMatrix);
                    colorShader->SetUniform4fv("model", model);
                    colorShader->SetUniform4f("color", glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
                    renderer->DrawBoundingBox(it->second->entity->model->boundingBox);
                }
            }
        }

        // Draw GUI
        gui.NewFrame();
        gui.PerformanceInfoOverlay(renderer, Info); // TODO: switch to string rendering
        gui.MakePanel(10.f, 10.f);
        gui.GeneralSettings(&gs);
        gui.EndPanel();
        gui.Draw();
        
        // Swap buffer
        Window->SwapBuffer();
    }

    /********************************************************
     *                                                      *
     *                 NOTE: Cleaning                       *
     *                                                      *
     ********************************************************/

    delete ReferenceGrid;
    delete AimPlayerRay;

    delete sCache;
    delete renderer;
    delete camera;
}
