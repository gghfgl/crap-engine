#include "editor.h"
#include "editor_gui.h"

const uint32 g_GroundMaxResolution = 50;
const uint32 g_ReferenceGridResolution = 50;

const float32 g_PickingSphereRadius = 0.5f; // Used for draw sphere and ray intersection

void RunEditorMode(Window *Window, InputState *Input, PlateformInfo *Info)
{
    // Init minimum stuff needed
    Camera *camera = new Camera((float32)Window->getWidth(), (float32)Window->getHeight(), glm::vec3(0.0f, 5.0f, 10.0f));
    Renderer *renderer = new Renderer();
    EditorGui gui = EditorGui(Window);

    // Compile and cache shaders
    ShaderCache *sCache = new ShaderCache();
    int32 error = sCache->compileShadersFromDirectory("./shaders", camera->projectionMatrix);
    if (error != 0)
    {
        Log::error("EXITCODE:111 Failed to compile shaders");
        exit(111);
    }

    // =================================================

    // Global states tracking
    GlobalState gs;
    gs.selectedModules = new std::map<uint32, Module*>;

    // Construct ReferenceGrid mesh
    std::vector<uint32> uEmpty;
    std::vector<Texture> tEmpty;
    std::vector<Vertex> vReferenceGrid(g_ReferenceGridResolution * 4 + 4, Vertex());
    Mesh *MeshReferenceGrid = new Mesh(vReferenceGrid, uEmpty, tEmpty);

    // Construct OriginDebug mesh
    std::vector<Vertex> vOriginDebug(6, Vertex());
    Mesh *MeshOriginDebug = new Mesh(vOriginDebug, uEmpty, tEmpty);

    // Construct Grounds, Skyboxes and Modules maps
    std::map<uint32, Ground*> *Grounds = new std::map<uint32, Ground*>;
    std::map<uint32, Skybox*> *Skyboxes = new std::map<uint32, Skybox*>;
    std::map<uint32, Module*> *Modules = new std::map<uint32, Module*>;

    // =================================================

    // Prepare static data rendering
    renderer->prepareOriginDebug(MeshOriginDebug);
    renderer->prepareReferenceGridSubData(MeshReferenceGrid, g_ReferenceGridResolution);

    // =============================

    while (gs.active)
    {
        Window->updateTime();
        Window->pollEvents();

        /********************************************************
         *                                                      *
         *        NOTE: I/O Keyboard and Mouse          *
         *                                                      *
         ********************************************************/

        if (Input->keyboard->isPressed[keyboard::CRAP_KEY_ESCAPE])
            gs.active = false;

        if (!gui.activeWindow)
        {
            if (Input->keyboard->isPressed[keyboard::CRAP_KEY_W])
                camera->processMovementDirection(FORWARD, Window->time->deltaTime);
            if (Input->keyboard->isPressed[keyboard::CRAP_KEY_S])
                camera->processMovementDirection(BACKWARD, Window->time->deltaTime);
            if (Input->keyboard->isPressed[keyboard::CRAP_KEY_A])
                camera->processMovementDirection(LEFT, Window->time->deltaTime);
            if (Input->keyboard->isPressed[keyboard::CRAP_KEY_D])
                camera->processMovementDirection(RIGHT, Window->time->deltaTime);
            if (Input->keyboard->isPressed[keyboard::CRAP_KEY_SPACE])
                camera->processMovementDirection(UP, Window->time->deltaTime);
            if (Input->keyboard->isPressed[keyboard::CRAP_KEY_LEFT_CONTROL])
                camera->processMovementDirection(DOWN, Window->time->deltaTime);

            if (Input->mouse->scrollOffsetY != 0.0f)
            {
                if (Input->getMouseScrollOffsetY() > 0)
                    camera->processMovementDirection(FORWARD, Window->time->deltaTime, 10.0f);
                else
                    camera->processMovementDirection(BACKWARD, Window->time->deltaTime, 10.0f);
            }
        }

        if (Input->mouse->leftButton)
        {
            if (gs.hoveredModule != 0)
                gs.selectedModuleIndex = gs.hoveredModule;
            else if (gs.dragModule == 0 && !gui.activeWindow)
                gs.selectedModuleIndex = 0;

            if (gs.selectedModuleIndex != 0 && !gui.activeWindow && gs.dragModule == 0)
                gs.dragModule = gs.selectedModuleIndex;

            if (!gui.activeWindow && !gs.selectedModuleIndex)
            {
                Input->updateMouseOffsets();
                camera->processMovementAngles(Input->mouse->offsetX, Input->mouse->offsetY);
            }
        }
        else
            gs.dragModule = 0;

        /********************************************************
         *                                                      *
         *                 NOTE: Simulate World                 *
         *                                                      *
         ********************************************************/

        glm::vec3 rayWorld = MouseRayDirectionWorld((float32)Input->mouse->posX,
                                                    (float32)Input->mouse->posY,
                                                    Window->getWidth(),
                                                    Window->getHeight(),
                                                    camera->projectionMatrix,
                                                    camera->getViewMatrix());

        // ground slider
        if (gs.currentGroundIndex != 0 && Grounds->find(gs.currentGroundIndex)->second->diffResolutionBuffer())
            Grounds->find(gs.currentGroundIndex)->second->isGenerated = false;

        // mouse ray intersection with modules (OBB)
        if (!Input->mouse->leftButton)
        {
            if (gs.drawFilter == MODULES_FILTER)
            {
                for (auto it = gs.selectedModules->begin(); it != gs.selectedModules->end(); it++)
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

                        if (TestRayOBBIntersection(camera->position,
                                                   rayWorld,
                                                   AABBmin,
                                                   AABBmax,
                                                   model,
                                                   rayIntersection))
                        {
                            gs.hoveredModule = it->first;
                            break;
                        }
                        else
                            gs.hoveredModule = 0;
                    }
                }
            }
        }
          
        glm::vec3 pIntersection = glm::vec3(0.0f);
        if (!RayPlaneIntersection(camera->position,
                                  rayWorld, glm::vec3(0.0f),
                                  glm::vec3(0.0f, 1.0f, 0.0f),
                                  &pIntersection))
            pIntersection = glm::vec3(0.0f);

        /********************************************************
         *                                                      *
         *                 NOTE: Rendering                      *
         *                                                      *
         ********************************************************/

        renderer->resetStats();
        renderer->newContext();
        glm::mat4 viewMatrix = camera->getViewMatrix();

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
        if (gs.drawFilter == ENVIRONMENT_FILTER && gs.currentGroundIndex != 0)
        {
            if (Grounds->find(gs.currentGroundIndex)->second->entity->model != nullptr)
            {
                Ground *selectedGround = Grounds->find(gs.currentGroundIndex)->second;
                if (!selectedGround->isGenerated)
                {
                    selectedGround->clearInstance();
                    selectedGround->updateModelMatrices();
                    selectedGround->instanceBufferID = renderer->prepareInstance(selectedGround->entity->model,
                                                                                 selectedGround->modelMatrices,
                                                                                 selectedGround->resolution * selectedGround->resolution);
                    selectedGround->isGenerated = true;
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
        if (gs.drawFilter == MODULES_FILTER)
        {
            for (auto it = gs.selectedModules->begin(); it != gs.selectedModules->cend(); it++)
            {
                if (it->second->entity->model != nullptr)
                {
                    Shader *defaultShader = sCache->getShader("default");
                    defaultShader->useProgram();
                    defaultShader->setUniform4fv("view", viewMatrix);
                    defaultShader->setUniform4fv("model", glm::mat4(1.0f));

                    bool isSelected = false;
                    if (gs.hoveredModule == it->first || gs.selectedModuleIndex == it->first)
                        isSelected = true;

                    if (gs.dragModule == it->first)
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
        if (gs.drawFilter == ENVIRONMENT_FILTER && gs.currentSkyboxIndex != 0)
        {
            if (Skyboxes->find(gs.currentSkyboxIndex)->second->directory.length() > 0)
            {
                Shader *skyboxShader = sCache->getShader("skybox");
                skyboxShader->useProgram();
                skyboxShader->setUniform4fv("view", glm::mat4(glm::mat3(viewMatrix))); // remove translation from the view matrix
                renderer->drawSkybox(Skyboxes->find(gs.currentSkyboxIndex)->second);
            }
        }

        // Draw GUI
        gui.newFrame();
        gui.performanceInfoOverlay(renderer, Info); // TODO: switch to string rendering
        gui.globalStatesOverlay(&gs);
        gui.makePanel(10.f, 10.f);
        gui.windowAndInputSettings(Input);
        gui.cameraSettings(camera);
        gui.drawSettings(gs.drawFilter);
        gui.groundSettings(gs.currentGroundIndex,
                           Grounds,
                           g_GroundMaxResolution);
        gui.skyboxSettings(gs.currentSkyboxIndex,
                           Skyboxes);
        gui.moduleSettings(gs.selectedModules,
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

    delete gs.selectedModules;

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
