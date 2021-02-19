#include "global.h"
#include "editor.h"
#include "editor_gui.h"

const char* defaultGroundList = "./assets/lists/crap_grounds_default.list";
const char* defaultSkyboxList = "./assets/lists/crap_skyboxes_default.list";
const char* defaultModuleList = "./assets/lists/crap_modules_default.list";

const uint32 g_GroundMaxResolution = 50;
const uint32 g_ReferenceGridResolution = 50;

void RunEditor(Window *Window, InputState *Input, PlateformInfo *Info, GlobalState *GlobalState)
{
    // Init camera
    Camera *camera = new Camera(
        glm::vec3(0.0f, 5.0f, 10.0f), // position
        glm::vec3(0.0f, 5.0f, 10.0f),   // lookAt
        glm::vec3(0.0f, 1.0f, 0.0f),   // worldUp
        45.0f,                         // fov
        (float32)Window->getWidth() / (float32)Window->getHeight(), // aspect
        0.1f, 100.0f);                 // near plane & far plane

    // Init renderer
    Renderer *renderer = new Renderer();

    // Init GUI
    EditorGui gui = EditorGui(Window);

    // Compile and cache shaders
    ShaderCache *sCache = new ShaderCache();
    int32 error = sCache->compileShadersFromDirectory("./shaders", camera->m_projectionMatrix);
    if (error != 0)
    {
        Log::error("EXITCODE:111 Failed to compile shaders");
        exit(111);
    }

    // =================================================

    // Global states
    EditorState es;
    es.selectedModules = new std::map<uint32, Module*>;

    // ReferenceGrid mesh
    std::vector<uint32> uEmpty;
    std::vector<Texture> tEmpty;
    std::vector<Vertex> vReferenceGrid(g_ReferenceGridResolution * 4 + 4, Vertex());
    Mesh *MeshReferenceGrid = new Mesh(vReferenceGrid, uEmpty, tEmpty);

    // OriginDebug mesh
    std::vector<Vertex> vOriginDebug(6, Vertex());
    Mesh *MeshOriginDebug = new Mesh(vOriginDebug, uEmpty, tEmpty);

    // Grounds, Skyboxes and Modules maps
    std::map<uint32, Ground*> *Grounds = new std::map<uint32, Ground*>;
    std::map<uint32, Skybox*> *Skyboxes = new std::map<uint32, Skybox*>;
    std::map<uint32, Module*> *Modules = new std::map<uint32, Module*>;

    // Opening default assets from list files
    error = OpenGroundListFromFile(defaultGroundList, Grounds);
    if (error != 0)
    {
        Log::error("\terror while opening file: %s\n", defaultGroundList);
        GlobalState->currentMode = EXIT_MODE;
        return;
    }

    error = OpenSkyboxListFromFile(defaultSkyboxList, Skyboxes);
    if (error != 0)
    {
        Log::error("\terror while opening file: %s\n", defaultSkyboxList);
        GlobalState->currentMode = EXIT_MODE;
        return;
    }

    error = OpenModuleListFromFile(defaultModuleList, Modules);
    if (error != 0)
    {
        Log::error("\terror while opening file: %s", defaultModuleList);
        GlobalState->currentMode = EXIT_MODE;
        return;
    }

    // =================================================

    // Prepare static data rendering
    renderer->prepareOriginDebug(MeshOriginDebug);
    renderer->prepareReferenceGridSubData(MeshReferenceGrid, g_ReferenceGridResolution);

    // =============================

    while (GlobalState->currentMode == EDITOR_MODE)
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

        if (Input->keyboard->isPressed[keyboard::CRAP_KEY_G])
            GlobalState->currentMode = GAME_MODE;

        if (!gui.activeWindow)
        {
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
                if (Input->getMouseScrollOffsetY() > 0)
                    camera->UpdatePositionFromDirection(CAMERA_FORWARD, Window->time->deltaTime, 10.0f);
                else
                    camera->UpdatePositionFromDirection(CAMERA_BACKWARD, Window->time->deltaTime, 10.0f);
            }
        }

        if (Input->mouse->leftButton)
        {
            if (es.hoveredModule != 0)
                es.selectedModuleIndex = es.hoveredModule;
            else if (es.dragModule == 0 && !gui.activeWindow)
                es.selectedModuleIndex = 0;

            if (es.selectedModuleIndex != 0 && !gui.activeWindow && es.dragModule == 0)
                es.dragModule = es.selectedModuleIndex;

            if (!gui.activeWindow && !es.selectedModuleIndex)
            {
                Input->updateMouseOffsets();
                camera->UpdatePositionFromAngle(Input->mouse->offsetX, Input->mouse->offsetY);
            }
        }
        else
            es.dragModule = 0;

        /********************************************************
         *                                                      *
         *                 NOTE: Simulate World                 *
         *                                                      *
         ********************************************************/

        // compute ray world from mouse
        glm::vec3 rayWorld = MouseRayDirectionWorld((float32)Input->mouse->posX,
                                                    (float32)Input->mouse->posY,
                                                    Window->getWidth(),
                                                    Window->getHeight(),
                                                    camera->m_projectionMatrix,
                                                    camera->m_viewMatrix);

        // mouse ray intersection with modules (OBB)
        if (!Input->mouse->leftButton)
        {
            if (es.drawFilter == MODULES_FILTER)
            {
                for (auto it = es.selectedModules->begin(); it != es.selectedModules->end(); it++)
                {
                    if (it->second->entity->model != nullptr)
                    {
                        float32 rayIntersection = 0.0f;
                        glm::mat4 model = glm::mat4(1.0f);
                        glm::vec3 AABBmin = glm::vec3(
                            it->second->entity->position.x - float32(it->second->entity->scale) / 2,
                            0.0f,
                            it->second->entity->position.z + float32(it->second->entity->scale) / 2);
                        glm::vec3 AABBmax = glm::vec3(
                            it->second->entity->position.x + float32(it->second->entity->scale) / 2,
                            float32(it->second->entity->scale),
                            it->second->entity->position.z - float32(it->second->entity->scale) / 2);

                        if (TestRayOBBIntersection(camera->m_position,
                                                   rayWorld,
                                                   AABBmin,
                                                   AABBmax,
                                                   model,
                                                   rayIntersection))
                        {
                            es.hoveredModule = it->first;
                            break;
                        }
                        else
                            es.hoveredModule = 0;
                    }
                }
            }
        }
          
        glm::vec3 pIntersection = glm::vec3(0.0f);
        if (!RayPlaneIntersection(camera->m_position,
                                  rayWorld, glm::vec3(0.0f),
                                  glm::vec3(0.0f, 1.0f, 0.0f),
                                  &pIntersection))
            pIntersection = glm::vec3(0.0f);

        // disable lookAt
        camera->SetCameraView(camera->m_position, camera->m_position, camera->m_worldUp);


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
        colorShader->setUniform4f("color", glm::vec4(0.360f, 0.360f, 0.360f, 1.0f));
        renderer->drawLines(MeshReferenceGrid, 1.0f);

        // Draw DebugOrigin
        colorShader->setUniform4f("color", glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
        renderer->drawLines(MeshOriginDebug, 2.0f);

        // Draw selected Ground
        if (es.drawFilter == ENVIRONMENT_FILTER && es.currentGroundIndex != 0)
        {
            if (Grounds->find(es.currentGroundIndex)->second->entity->model != nullptr)
            {
                Ground *selectedGround = Grounds->find(es.currentGroundIndex)->second;
                if (selectedGround->diffResolutionBuffer())
                {
                    selectedGround->clearInstance();
                    selectedGround->updateModelMatrices();
                    selectedGround->instanceBufferID = renderer->prepareInstance(selectedGround->entity->model,
                                                                                 selectedGround->modelMatrices,
                                                                                 selectedGround->resolution * selectedGround->resolution);
                }

                Shader *instancedShader = sCache->getShader("instanced");
                instancedShader->useProgram();
                instancedShader->setUniform4fv("view", viewMatrix);
                instancedShader->setUniform4fv("model", glm::mat4(1.0f));
                renderer->drawInstanceModel(selectedGround->entity->model,
                                            instancedShader,
                                            selectedGround->resolution * selectedGround->resolution);
            }
        }
                  
        // Draw Modules
        if (es.drawFilter == MODULES_FILTER)
        {
            for (auto it = es.selectedModules->begin(); it != es.selectedModules->cend(); it++)
            {
                if (it->second->entity->model != nullptr)
                {
                    Shader *defaultShader = sCache->getShader("default");
                    defaultShader->useProgram();
                    defaultShader->setUniform4fv("view", viewMatrix);
                    defaultShader->setUniform4fv("model", glm::mat4(1.0f));

                    bool isSelected = false;
                    if (es.hoveredModule == it->first || es.selectedModuleIndex == it->first)
                        isSelected = true;

                    if (es.dragModule == it->first)
                        it->second->entity->position = pIntersection;

                    // Model
                    glm::mat4 model = glm::mat4(1.0f);
                    model = glm::translate(model, glm::vec3(it->second->entity->position.x,
                                                            it->second->entity->position.y + float32(it->second->entity->scale) / 2,
                                                            it->second->entity->position.z));
                    model = glm::scale(model, glm::vec3(it->second->entity->scale));
                    model = glm::rotate(model, glm::radians(it->second->entity->rotate), glm::vec3(0.0f, 1.0f, 0.0f));
                    defaultShader->setUniform4fv("model", model);
                    renderer->drawModel(it->second->entity->model, defaultShader);

                    if (isSelected)
                    {
                        Shader *outlineShader = sCache->getShader("outline");
                        outlineShader->useProgram();
                        outlineShader->setUniform4fv("view", viewMatrix);
                        outlineShader->setUniform4fv("model", glm::mat4(1.0f));

                        float32 scaleModifier = (float32(it->second->entity->scale) + 0.1f) / it->second->entity->scale;
                        model = glm::scale(model, glm::vec3(scaleModifier));
                        outlineShader->setUniform4fv("model", model);

                        renderer->drawModelOutline(it->second->entity->model, outlineShader);
                    }
                }
            }
        }

        // Draw selected Skybox
        if (es.drawFilter == ENVIRONMENT_FILTER && es.currentSkyboxIndex != 0)
        {
            if (Skyboxes->find(es.currentSkyboxIndex)->second->directory.length() > 0)
            {
                Shader *skyboxShader = sCache->getShader("skybox");
                skyboxShader->useProgram();
                skyboxShader->setUniform4fv("view", glm::mat4(glm::mat3(viewMatrix))); // remove translation from the view matrix
                renderer->drawSkybox(Skyboxes->find(es.currentSkyboxIndex)->second);
            }
        }

        // Draw GUI
        gui.newFrame();
        gui.performanceInfoOverlay(renderer, Info); // TODO: switch to string rendering
        gui.editorStatesOverlay(&es);
        gui.makePanel(10.f, 10.f);
        gui.windowAndInputSettings(Input);
        gui.cameraSettings(camera);
        gui.drawSettings(es.drawFilter);
        gui.groundSettings(es.currentGroundIndex,
                           Grounds,
                           g_GroundMaxResolution);
        gui.skyboxSettings(es.currentSkyboxIndex,
                           Skyboxes);
        gui.moduleSettings(es.selectedModules,
                           Modules);
        gui.endPanel();
        gui.draw();

        // Swap buffer
        Window->swapBuffer();
    }

    /********************************************************
     *                                                      *
     *                 NOTE: Cleaning                       *
     *                                                      *
     ********************************************************/

    delete es.selectedModules;

    for (auto it = Modules->begin(); it != Modules->end(); it++)
        delete it->second;
    delete Modules;

    for (auto it = Grounds->begin(); it != Grounds->end(); it++)
        delete it->second;
    delete Grounds;

    for (auto it = Skyboxes->begin(); it != Skyboxes->end(); it++)
        delete it->second;
    delete Skyboxes;

    delete MeshReferenceGrid;
    delete MeshOriginDebug;

    delete sCache;
    delete renderer;
    delete camera;
}
