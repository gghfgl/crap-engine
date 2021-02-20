#include "global.h"
#include "game.h"

void RunGame(Window *Window, InputState *Input, GlobalState *GlobalState)
{
    // Init camera
    Camera *camera = new Camera(
        glm::vec3(0.0f, 30.0f, 20.0f), // position
        glm::vec3(0.0f, 0.0f, 0.0f),   // lookAt
        glm::vec3(0.0f, 1.0f, 0.0f),   // worldUp
        45.0f,                         // fov
        -60.0f,                          // pitch
        (float32)Window->getWidth() / (float32)Window->getHeight(), // aspect
        0.1f, 100.0f);                 // near plane & far plane

    // Init renderer
    Renderer *renderer = new Renderer();

    // Compile and cache shaders
    ShaderCache *sCache = new ShaderCache();
    int32 error = sCache->compileShadersFromDirectory("./shaders", camera->m_projectionMatrix);
    if (error != 0)
    {
        Log::error("EXITCODE:111 Failed to compile shaders");
        exit(111);
    }

    // =================================================

    // ReferenceGrid
    std::vector<uint32> uEmpty;
    std::vector<Texture> tEmpty;
    std::vector<Vertex> vReferenceGrid(g_ReferenceGridResolution * 4 + 4, Vertex());
    Mesh *ReferenceGrid = new Mesh(vReferenceGrid, uEmpty, tEmpty);

    // Aim ray
    std::vector<Vertex> vRay(2, Vertex());
    Mesh *AimPlayerRay = new Mesh(vRay, uEmpty, tEmpty);

    // Test Payer
    Player *testPlayer = new Player("testPlayer", "./assets/models/terrain/untitled.obj", glm::vec3(0.0f));

    // Modules environment
    std::map<uint32, Module*> *Modules = new std::map<uint32, Module*>;
    error = OpenModuleListFromFile(defaultModuleList, Modules);
    if (error != 0)
    {
        Log::error("\terror while opening file: %s", defaultModuleList);
        GlobalState->currentMode = EXIT_MODE;
        return;
    }

    // =================================================

    // Prepare static data rendering
    renderer->prepareReferenceGridSubData(ReferenceGrid, g_ReferenceGridResolution);

    // =============================

    while (GlobalState->currentMode == GAME_MODE)
    {
        Window->updateTime();
        Window->pollEvents();

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
            testPlayer->UpdatePositionFromDirection(ENTITY_FORWARD, Window->time->deltaTime);
        if (Input->keyboard->isPressed[keyboard::CRAP_KEY_S])
            testPlayer->UpdatePositionFromDirection(ENTITY_BACKWARD, Window->time->deltaTime);
        if (Input->keyboard->isPressed[keyboard::CRAP_KEY_A])
            testPlayer->UpdatePositionFromDirection(ENTITY_LEFT, Window->time->deltaTime);
        if (Input->keyboard->isPressed[keyboard::CRAP_KEY_D])
            testPlayer->UpdatePositionFromDirection(ENTITY_RIGHT, Window->time->deltaTime);

        if (Input->mouse->scrollOffsetY != 0.0f)
        {
            if (Input->getMouseScrollOffsetY() > 0)
                camera->UpdatePositionFromDirection(CAMERA_FORWARD, Window->time->deltaTime, 30.0f);
            else
                camera->UpdatePositionFromDirection(CAMERA_BACKWARD, Window->time->deltaTime, 30.0f);
        }

        // if (Input->mouse->leftButton)
        // {
        //     Input->updateMouseOffsets();
        //     camera->UpdateArcballFromAngle(Input->mouse->offsetX, Input->mouse->offsetY);
        // }

        /********************************************************
         *                                                      *
         *                    NOTE: Simulate                    *
         *                                                      *
         ********************************************************/

        // Camera follow player
        glm::vec3 behindPlayer = glm::vec3(testPlayer->entity->position.x, camera->m_position.y, testPlayer->entity->position.z + 20.0f);
        camera->SetCameraView(behindPlayer, testPlayer->entity->position, camera->m_worldUp);

        // Mouse ray
        glm::vec3 mouseRayWorld = MouseRayDirectionWorld((float32)Input->mouse->posX,
                                                    (float32)Input->mouse->posY,
                                                    Window->getWidth(),
                                                    Window->getHeight(),
                                                    camera->m_projectionMatrix,
                                                    camera->m_viewMatrix);

        // Mouse ray intersec with plane
        glm::vec3 mouseRayIntersection = glm::vec3(0.0f);
        if (!RayPlaneIntersection(camera->m_position,
                                  mouseRayWorld, glm::vec3(0.0f),
                                  glm::vec3(0.0f, 1.0f, 0.0f),
                                  &mouseRayIntersection))
            mouseRayIntersection = glm::vec3(0.0f);

        /********************************************************
         *                                                      *
         *                 NOTE: Rendering                      *
         *                                                      *
         ********************************************************/

        // Clear renderer
        renderer->resetStats();
        renderer->newContext();
        glm::mat4 viewMatrix = camera->m_viewMatrix;

        // Draw reference grid
        Shader *colorShader = sCache->getShader("color");
        colorShader->useProgram();
        colorShader->setUniform4fv("view", viewMatrix);
        colorShader->setUniform4fv("model", glm::mat4(1.0f));

        colorShader->setUniform4f("color", glm::vec4(0.360f, 1.0f, 0.360f, 1.0f));
        renderer->drawLines(ReferenceGrid, 1.0f);
        
        // Draw mouse ray
        renderer->prepareRaySubData(AimPlayerRay, testPlayer->entity->position, mouseRayIntersection);
        colorShader->setUniform4f("color", glm::vec4(1.0f, 0.8f, 0.0f, 1.0));
        renderer->drawLines(AimPlayerRay, 1.0f);

        // Draw player
        Shader *defaultShader = sCache->getShader("default");
        defaultShader->useProgram();
        defaultShader->setUniform4fv("view", viewMatrix);
        defaultShader->setUniform4fv("model", glm::mat4(1.0f));

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(testPlayer->entity->position.x,
                                                testPlayer->entity->position.y + float32(testPlayer->entity->scale) / 2,
                                                testPlayer->entity->position.z));
        model = glm::scale(model, glm::vec3(testPlayer->entity->scale));
        model = glm::rotate(model, glm::radians(testPlayer->entity->rotate), glm::vec3(0.0f, 1.0f, 0.0f));
        defaultShader->setUniform4fv("model", model);
        //renderer->drawModel(testPlayer->entity->model, defaultShader);
        
        // Draw modules
        for (auto it = Modules->begin(); it != Modules->cend(); it++)
        {
            if (it->second->entity->model != nullptr)
            {
                Shader *defaultShader = sCache->getShader("default");
                defaultShader->useProgram();
                defaultShader->setUniform4fv("view", viewMatrix);
                defaultShader->setUniform4fv("model", glm::mat4(1.0f));

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
                defaultShader->setUniform4fv("model", model);
                renderer->drawModel(it->second->entity->model, defaultShader);

                // if (isSelected)
                // {
                //     Shader *outlineShader = sCache->getShader("outline");
                //     outlineShader->useProgram();
                //     outlineShader->setUniform4fv("view", viewMatrix);
                //     outlineShader->setUniform4fv("model", glm::mat4(1.0f));

                //     float32 scaleModifier = (float32(it->second->entity->scale) + 0.1f) / it->second->entity->scale;
                //     model = glm::scale(model, glm::vec3(scaleModifier));
                //     outlineShader->setUniform4fv("model", model);

                //     renderer->drawModelOutline(it->second->entity->model, outlineShader);
                // }
            }
        }

        // Swap buffer
        Window->swapBuffer();
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
