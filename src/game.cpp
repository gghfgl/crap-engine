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
    // Player *testPlayer = new Player("testPlayer", "./assets/models/untitled-scene-obj/untitled.obj", glm::vec3(0.0f));
    Player *testPlayer = new Player("testPlayer", "./assets/models/vampire/dancing_vampire.dae", glm::vec3(0.0f));
    Animation testAnimation = Animation("./assets/models/vampire/dancing_vampire.dae", testPlayer->entity->model);
    Animator testAnimator(&testAnimation);

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

        // TODO: @animation
        //testAnimator.UpdateAnimation(Window->time->deltaTime);
        //testAnimator.PlayAnimation(&testAnimation);

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

        // // Draw Player
        // Shader *defaultShader = sCache->GetShader("default");
        // defaultShader->UseProgram();
        // defaultShader->SetUniform4fv("view", viewMatrix);
        // defaultShader->SetUniform4fv("model", glm::mat4(1.0f));
        
        // glm::mat4 model = glm::mat4(1.0f);
        // model = glm::translate(model, testPlayer->entity->position);
        // model = glm::scale(model, glm::vec3(testPlayer->entity->scale));
        // model = glm::rotate(model, glm::radians(testPlayer->entity->rotate), glm::vec3(0.0f, 1.0f, 0.0f));
        // defaultShader->SetUniform4fv("model", model);
        // renderer->DrawModel(testPlayer->entity->model, defaultShader);

        // DEBUG ==========================================================================================
        Shader *animateShader = sCache->GetShader("animate");
        animateShader->UseProgram();
        animateShader->SetUniform4fv("view", viewMatrix);
        animateShader->SetUniform4fv("model", glm::mat4(1.0f));

        auto transforms = testAnimator.GetPoseTransforms();
        for (int i = 0; i < transforms.size(); ++i)
        {
            std::string bm = "finalBonesMatrices[" + std::to_string(i) + "]";
            animateShader->SetUniform4fv(bm.c_str(), transforms[i]);
        }
        
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, testPlayer->entity->position);
        model = glm::scale(model, glm::vec3(.05f, .05f, .05f));
        model = glm::rotate(model, glm::radians(testPlayer->entity->rotate), glm::vec3(0.0f, 1.0f, 0.0f));
        animateShader->SetUniform4fv("model", model);
        renderer->DrawModel(testPlayer->entity->model, animateShader);
        // DEBUG ==========================================================================================
        
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

                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, it->second->entity->position);
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
    delete testPlayer;
    delete AimPlayerRay;
    for (auto it = Modules->begin(); it != Modules->end(); it++)
        delete it->second;
    delete Modules;

    delete sCache;
    delete renderer;
    delete camera;
}
