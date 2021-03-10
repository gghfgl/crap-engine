#include "global.h"
#include "game.h"
#include "game_gui.h"

const uint32 g_GroundMaxResolution = 50;
const uint32 g_ReferenceGridResolution = 50;

void RunGame(Window *Window, InputState *Input, PlateformInfo *Info, GlobalState *GlobalState)
{
    // Init game state
    GameState gs;

    // Init renderer
    Renderer *renderer = new Renderer();
    // Init camera
    Camera *camera = new Camera(
        glm::vec3(0.0f, 30.0f, 20.0f), // position
        glm::vec3(0.0f, 0.0f, 0.0f),   // lookAt
        glm::vec3(0.0f, 1.0f, 0.0f),   // worldUp
        45.0f,                         // fov
        -60.0f,                        // pitch
        (float32)Window->GetWidth() / (float32)Window->GetHeight(), // aspectRatio
        gs.nearPlane, gs.farPlane);    // near plane & far plane
    
    // Compile and cache shaders
    ShaderCache *sCache = new ShaderCache();
    int32 error = sCache->CompileShadersFromDirectory("./shaders");
    if (error != 0)
    {
        Log::error("EXITCODE:111 Failed to compile shaders");
        exit(111);
    }

    // =================================================

    // // ReferenceGrid
    std::vector<uint32> indicesEmpty; // TODO: init default in constructor?
    std::vector<Texture> texturesEmpty;
    std::vector<Vertex> vReferenceGrid(g_ReferenceGridResolution * 4 + 4, Vertex());
    Mesh *ReferenceGrid = new Mesh(vReferenceGrid, indicesEmpty, texturesEmpty);
    renderer->PrepareReferenceGridSubData(ReferenceGrid, g_ReferenceGridResolution);

    // Test Payer
    //Player *testPlayer = new Player("testPlayer", "./assets/models/untitled-scene-obj/untitled.obj", glm::vec3(0.0f));

    Model *testModel = new Model("./assets/models/vampire/dancing_vampire.dae");
    Animation *testAnimation = new Animation("./assets/models/vampire/dancing_vampire.dae", testModel);
    // DEBUG
    // for (auto it = testModel->jointTransforms.begin(); it != testModel->jointTransforms.end(); it++)
    //     Log::debug("name=%s\t ID=%d\n", it->first.c_str(), it->second.ID);
    // Log::debug("name=%s\t count=%d\n", testAnimation->rootNode.name.c_str(), testAnimation->rootNode.childrenCount);
    // for (auto& child : testAnimation->rootNode.children)
    //     Log::debug("name=%s\t count=%d size=%d\n", child.name.c_str(), child.childrenCount, child.children.size());
    // for (auto& joint : testModel->joints)
    //     Log::debug("name=%s\n", joint->name.c_str());

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
            camera->UpdatePositionFromDirection(CAMERA_FORWARD, Window->time->deltaTime);
        if (Input->keyboard->isPressed[keyboard::CRAP_KEY_S])
            camera->UpdatePositionFromDirection(CAMERA_BACKWARD, Window->time->deltaTime);
        if (Input->keyboard->isPressed[keyboard::CRAP_KEY_A])
            camera->UpdatePositionFromDirection(CAMERA_LEFT, Window->time->deltaTime);
        if (Input->keyboard->isPressed[keyboard::CRAP_KEY_D])
            camera->UpdatePositionFromDirection(CAMERA_RIGHT, Window->time->deltaTime);
        if (Input->keyboard->isPressed[keyboard::CRAP_KEY_SPACE])
            camera->UpdatePositionFromDirection(CAMERA_UP, Window->time->deltaTime);
        if (Input->keyboard->isPressed[keyboard::CRAP_KEY_LEFT_CONTROL])
            camera->UpdatePositionFromDirection(CAMERA_DOWN, Window->time->deltaTime);

        if (Input->mouse->scrollOffsetY != 0.0f)
        {
            if (Input->GetMouseScrollOffsetY() > 0)
                camera->UpdatePositionFromDirection(CAMERA_FORWARD, Window->time->deltaTime, 10.0f);
            else
                camera->UpdatePositionFromDirection(CAMERA_BACKWARD, Window->time->deltaTime, 10.0f);
        }

        if (Input->mouse->leftButton)
        {
            Input->UpdateMouseOffsets(); // TODO: Meh
            camera->UpdatePositionFromAngle(Input->mouse->offsetX, Input->mouse->offsetY);
        }

        /********************************************************
         *                                                      *
         *                    NOTE: Simulate                    *
         *                                                      *
         ********************************************************/

        // Free look
        camera->SetCameraView(camera->position, camera->position, camera->worldUp);

        /********************************************************
         *                                                      *
         *                 NOTE: Rendering                      *
         *                                                      *
         ********************************************************/

        // Clear renderer
        renderer->ResetStats();
        renderer->NewContext();
        glm::mat4 viewMatrix = camera->viewMatrix;
        glm::mat4 projectionMatrix = camera->projectionMatrix;
        glm::mat4 identityMatrix = glm::mat4(1.0f);
        
        // Draw ReferenceGrid
        Shader *colorShader = sCache->GetShader("color");
        colorShader->UseProgram();
        colorShader->SetUniform4fv("projection", projectionMatrix);
        colorShader->SetUniform4fv("view", viewMatrix);
        colorShader->SetUniform4fv("model", identityMatrix);

        colorShader->SetUniform4f("color", glm::vec4(0.360f, 1.0f, 0.360f, 1.0f));
        renderer->DrawLines(ReferenceGrid, 1.0f);
        
        // // Draw Player
        // Shader *defaultShader = sCache->GetShader("default");
        // defaultShader->UseProgram();
        // defaultShader->SetUniform4fv("projection", projectionMatrix);
        // defaultShader->SetUniform4fv("view", viewMatrix);
        
        // glm::mat4 model = identityMatrix;
        // model = glm::translate(model, testPlayer->entity->position);
        // model = glm::scale(model, glm::vec3(testPlayer->entity->scale));
        // //model = glm::scale(model, glm::vec3(0.01f));
        // model = glm::rotate(model, glm::radians(testPlayer->entity->rotate), glm::vec3(0.0f, 1.0f, 0.0f));
        // defaultShader->SetUniform4fv("model", model);
        // renderer->DrawModel(testPlayer->entity->model, defaultShader);
        
        // Swap buffer
        Window->SwapBuffer();
    }

    /********************************************************
     *                                                      *
     *                 NOTE: Cleaning                       *
     *                                                      *
     ********************************************************/

    delete ReferenceGrid;
    // delete testPlayer;
    delete testModel;
    delete testAnimation;

    delete camera;
    delete sCache;
    delete renderer;
}
