#include "global.h"
#include "game.h"

void RunGame(Window *Window, InputState *Input, GlobalState *GlobalState)
{
    // Init camera
    Camera *camera = new Camera(
        glm::vec3(0.0f, 30.0f, 10.0f), // position
        glm::vec3(0.0f, 0.0f, 0.0f),   // lookAt
        glm::vec3(0.0f, 1.0f, 0.0f),   // worldUp
        45.0f,                         // fov
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

    // ReferenceGrid mesh
    std::vector<uint32> uEmpty;
    std::vector<Texture> tEmpty;
    std::vector<Vertex> vReferenceGrid(g_ReferenceGridResolution * 4 + 4, Vertex());
    Mesh *MeshReferenceGrid = new Mesh(vReferenceGrid, uEmpty, tEmpty);

    // Modules maps
    Player *testPlayer = new Player("testPlayer", "./assets/models/terrain/untitled.obj", glm::vec3(0.0f));

    // =================================================

    // Prepare static data rendering
    renderer->prepareReferenceGridSubData(MeshReferenceGrid, g_ReferenceGridResolution);

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
         *                 NOTE: Simulate World                 *
         *                                                      *
         ********************************************************/
        
        glm::vec3 behindPlayer = glm::vec3(testPlayer->entity->position.x, camera->m_position.y, testPlayer->entity->position.z + 20.0f);
        camera->SetCameraView(behindPlayer, testPlayer->entity->position, camera->m_worldUp);

        /********************************************************
         *                                                      *
         *                 NOTE: Rendering                      *
         *                                                      *
         ********************************************************/
        renderer->resetStats();
        renderer->newContext();
        glm::mat4 viewMatrix = camera->m_viewMatrix;

        Shader *colorShader = sCache->getShader("color");
        colorShader->useProgram();
        colorShader->setUniform4fv("view", viewMatrix);
        colorShader->setUniform4fv("model", glm::mat4(1.0f));

        // Draw ReferenceGrid
        colorShader->setUniform4f("color", glm::vec4(0.360f, 1.0f, 0.360f, 1.0f));
        renderer->drawLines(MeshReferenceGrid, 1.0f);

        // Draw Modules
        Shader *defaultShader = sCache->getShader("default");
        defaultShader->useProgram();
        defaultShader->setUniform4fv("view", viewMatrix);
        defaultShader->setUniform4fv("model", glm::mat4(1.0f));

        // Model
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(testPlayer->entity->position.x,
                                                testPlayer->entity->position.y + float32(testPlayer->entity->scale) / 2,
                                                testPlayer->entity->position.z));
        model = glm::scale(model, glm::vec3(testPlayer->entity->scale));
        model = glm::rotate(model, glm::radians(testPlayer->entity->rotate), glm::vec3(0.0f, 1.0f, 0.0f));
        defaultShader->setUniform4fv("model", model);
        renderer->drawModel(testPlayer->entity->model, defaultShader);

        // Swap buffer
        Window->swapBuffer();
    }

    /********************************************************
     *                                                      *
     *                 NOTE: Cleaning                       *
     *                                                      *
     ********************************************************/

    delete MeshReferenceGrid;

    delete sCache;
    delete renderer;
    delete camera;
}
