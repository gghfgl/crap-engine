#include "global.h"
#include "game.h"

const uint32 g_ReferenceGridResolution = 50;

void RunGame(Window *Window, InputState *Input, GlobalState *GlobalState)
{
    // Init camera
    Camera *camera = new Camera(
        glm::vec3(0.0f, 10.0f, 0.0f), // position
        glm::vec3(0.0f, 0.0f, 0.0f),   // lookAt
        glm::vec3(0.0f, 1.0f, 0.0f),   // upVector
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
    Module *testModule = new Module("testModule", "./assets/models/terrain/untitled.obj");
    testModule->entity->position = glm::vec3(0.0f, 0.0f, 0.0f);

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

        // if (Input->keyboard->isPressed[keyboard::CRAP_KEY_E])
        //     GlobalState->currentMode = EDITOR_MODE;

        if (Input->keyboard->isPressed[keyboard::CRAP_KEY_W])
            camera->UpdatePositionFromDirection(FORWARD, Window->time->deltaTime);
        if (Input->keyboard->isPressed[keyboard::CRAP_KEY_S])
            camera->UpdatePositionFromDirection(BACKWARD, Window->time->deltaTime);
        if (Input->keyboard->isPressed[keyboard::CRAP_KEY_A])
            camera->UpdatePositionFromDirection(LEFT, Window->time->deltaTime);
        if (Input->keyboard->isPressed[keyboard::CRAP_KEY_D])
            camera->UpdatePositionFromDirection(RIGHT, Window->time->deltaTime);
        if (Input->keyboard->isPressed[keyboard::CRAP_KEY_SPACE])
            camera->UpdatePositionFromDirection(UP, Window->time->deltaTime);
        if (Input->keyboard->isPressed[keyboard::CRAP_KEY_LEFT_CONTROL])
            camera->UpdatePositionFromDirection(DOWN, Window->time->deltaTime);

        if (Input->mouse->scrollOffsetY != 0.0f)
        {
            if (Input->getMouseScrollOffsetY() > 0)
                camera->UpdatePositionFromDirection(FORWARD, Window->time->deltaTime, 10.0f);
            else
                camera->UpdatePositionFromDirection(BACKWARD, Window->time->deltaTime, 10.0f);
        }

        if (Input->mouse->leftButton)
        {
            Input->updateMouseOffsets();
            camera->UpdatePositionFromAngle(Input->mouse->offsetX, Input->mouse->offsetY);
        }

        /********************************************************
         *                                                      *
         *                 NOTE: Simulate World                 *
         *                                                      *
         ********************************************************/
        glm::vec3 rayWorld = MouseRayDirectionWorld(Window->getWidth() / 2,
                                                    Window->getHeight() / 2,
                                                    Window->getWidth(),
                                                    Window->getHeight(),
                                                    camera->m_projectionMatrix,
                                                    camera->m_viewMatrix);

        glm::vec3 pIntersection = glm::vec3(0.0f);
        if (!RayPlaneIntersection(camera->m_position,
                                  rayWorld, glm::vec3(0.0f),
                                  glm::vec3(0.0f, 1.0f, 0.0f),
                                  &pIntersection))
            pIntersection = glm::vec3(0.0f);




        // TODO
        //camera->position.y = 20.0f;

        // camera/view transformation
        glm::mat4 viewTest = glm::mat4(1.0f);
        float radius = 10.0f;
        float camX   = sin(glfwGetTime()) * radius;
        float camZ   = cos(glfwGetTime()) * radius;
        // std::cout << camX << " - " << camZ << "\n";
        // viewTest = glm::lookAt(glm::vec3(testModule->entity->position.x+10.0f, 20.0f, testModule->entity->position.z), testModule->entity->position, glm::vec3(0.0f, 1.0f, 0.0f));

        glm::mat4 rot_mat = glm::rotate(glm::mat4(1.f), camera->m_yaw, glm::vec3(1,0,0));
        rot_mat = glm::rotate(glm::mat4(1.f), camera->m_pitch, glm::vec3(0,1,0)) * rot_mat;
        glm::vec3 aa = rot_mat * glm::vec4(testModule->entity->position - camera->m_position, 0.0f);
        glm::vec3 bb = camera->m_position + aa;
        viewTest = glm::lookAt(bb, testModule->entity->position, glm::vec3(0.0f, 1.0f, 0.0f));

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
        colorShader->setUniform4fv("view", viewTest);
        colorShader->setUniform4fv("model", glm::mat4(1.0f));

        // Draw ReferenceGrid
        colorShader->setUniform4f("color", glm::vec4(0.360f, 1.0f, 0.360f, 1.0f));
        renderer->drawLines(MeshReferenceGrid, 1.0f);

        // Draw Modules
        Shader *defaultShader = sCache->getShader("default");
        defaultShader->useProgram();
        defaultShader->setUniform4fv("view", viewTest);
        defaultShader->setUniform4fv("model", glm::mat4(1.0f));

        // TODO
        //testModule->entity->position = pIntersection;

        // Model
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(testModule->entity->position.x,
                                                testModule->entity->position.y + float32(testModule->entity->scale) / 2,
                                                testModule->entity->position.z));
        model = glm::scale(model, glm::vec3(testModule->entity->scale));
        model = glm::rotate(model, glm::radians(testModule->entity->rotate), glm::vec3(0.0f, 1.0f, 0.0f));
        defaultShader->setUniform4fv("model", model);
        renderer->drawModel(testModule->entity->model, defaultShader);

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
