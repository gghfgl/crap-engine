#include "editor.h"
#include "editor_gui.h"

void PrepareAxisDebug(Mesh *Mesh); // TODO: rename origin
void PushReferenceGridSubData(Mesh *Mesh, uint32 resolution);
void PushMouseRaySubData(Mesh *Mesh, glm::vec3 origin, glm::vec3 direction);

static uint32 g_CurrentGroundIndex = 0;
static uint32 g_CurrentSkyboxIndex = 0;
static uint32 g_CurrentModuleIndex = 0;

const uint32 g_GroundMaxResolution = 50;
const uint32 g_ReferenceGridResolution = 50;

// TODO: Clean mess below and replace it by a state machine struct?
static uint32 g_HoveredEntity = 0;
static uint32 g_SelectedEntity = 0;
static uint32 g_DragEntity = 0;

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
    if (error == -1)
        exit(111);

    // =================================================

    // Construct ReferenceGrid mesh
    std::vector<uint32> uEmpty;
    std::vector<Texture> tEmpty;
    std::vector<Vertex> vGrid(g_ReferenceGridResolution * 4 + 4, Vertex());
    Mesh *MeshGrid = new Mesh(vGrid, uEmpty, tEmpty);

    // Construct DebugOrigin mesh
    std::vector<Vertex> vAxisDebug(6, Vertex());
    Mesh *MeshAxisDebug = new Mesh(vAxisDebug, uEmpty, tEmpty);

    // Construct Grounds, Skyboxes and Modules maps
    std::map<uint32, Ground*> *Grounds = new std::map<uint32, Ground*>;
    std::map<uint32, Skybox*> *Skyboxes = new std::map<uint32, Skybox*>;
    std::map<uint32, Module*> *Modules = new std::map<uint32, Module*>;

    // =================================================

    // TODO: clean mess below | useless
    editor_t *Editor = new editor_t;
    Editor->Active = true;
    Editor->GridResolution = 0;
    Editor->MeshGrid = MeshGrid;
    Editor->MeshAxisDebug = MeshAxisDebug;
    Editor->ShowSkybox = false;

    // TODO: tmp wainting editor struct | move to entity?
    PrepareAxisDebug(Editor->MeshAxisDebug);
    PushReferenceGridSubData(Editor->MeshGrid, g_ReferenceGridResolution);

    // =============================

    while (Editor->Active)
    {
        Window->updateTime();
        Window->pollEvents();

        /********************************************************
         *                                                      *
         *        NOTE: Handler I/O Keyboard and Mouse          *
         *                                                      *
         ********************************************************/

        if (Input->keyboard->isPressed[keyboard::CRAP_KEY_ESCAPE])
            Editor->Active = false;

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
            if (g_HoveredEntity != 0)
                g_SelectedEntity = g_HoveredEntity;
            else if (g_DragEntity == 0 && !gui.activeWindow)
                g_SelectedEntity = 0;

            if (g_SelectedEntity != 0 && !gui.activeWindow)
                g_DragEntity = g_SelectedEntity;

            if (!gui.activeWindow && !g_SelectedEntity)
            {
                Input->updateMouseOffsets();
                camera->processMovementAngles(Input->mouse->offsetX, Input->mouse->offsetY);
            }
        }
        else
            g_DragEntity = 0;

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
        if (g_CurrentGroundIndex != 0 && Grounds->find(g_CurrentGroundIndex)->second->diffResolutionBuffer())
            Grounds->find(g_CurrentGroundIndex)->second->isGenerated = false;
        
        // mouse ray intersection sphere selector objects
        for (auto it = Modules->begin(); it != Modules->end(); it++)
        {
            float32 rayIntersection = 0.0f;
            glm::vec3 spherePos = glm::vec3(
                it->second->entity->position.x,
                it->second->entity->position.y,
                it->second->entity->position.z);

            if (RaySphereIntersection(camera->position,
                                      rayWorld,
                                      spherePos,
                                      g_PickingSphereRadius,
                                      &rayIntersection))
            {
                g_HoveredEntity = it->first;
                break;
            }
            else
                g_HoveredEntity = 0;
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
        renderer->drawLines(Editor->MeshGrid, 1.0f);

        // Draw DebugOrigin
        colorShader->setUniform4f("color", glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
        renderer->drawLines(Editor->MeshAxisDebug, 2.0f);

        // Draw selected Ground
        if (g_CurrentGroundIndex != 0)
        {
            if (Grounds->find(g_CurrentGroundIndex)->second->entity->model != nullptr)
            {
                Ground *selectedGround = Grounds->find(g_CurrentGroundIndex)->second;
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
        if (g_CurrentModuleIndex != 0)
        {
            if (Modules->find(g_CurrentModuleIndex)->second->entity->model != nullptr)
            {
                Shader *defaultShader = sCache->getShader("default");
                defaultShader->useProgram();
                defaultShader->setUniform4fv("view", viewMatrix);
                defaultShader->setUniform4fv("model", glm::mat4(1.0f));

                uint32 moduleID = Modules->find(g_CurrentModuleIndex)->first;
                Module *selectedModel = Modules->find(g_CurrentModuleIndex)->second;

                bool isSelected = false;
                if (g_HoveredEntity == moduleID || g_SelectedEntity == moduleID)
                    isSelected = true;

                if (g_DragEntity == moduleID)
                    selectedModel->entity->position = pIntersection;

                // Model
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, selectedModel->entity->position);
                model = glm::scale(model, glm::vec3(selectedModel->entity->scale));
                model = glm::rotate(model, glm::radians(selectedModel->entity->rotate), glm::vec3(0.0f, 1.0f, 0.0f));
                defaultShader->setUniform4fv("model", model);
                defaultShader->setUniform1ui("flip_color", isSelected);
                renderer->drawModel(selectedModel->entity->model, defaultShader);

                // // Picking sphere
                // model = glm::mat4(1.0f);
                // model = glm::translate(model, selectedModel->entity->position);
                // defaultShader->setUniform4fv("model", model);
                // defaultShader->setUniform1ui("flip_color", isSelected);
                // renderer->drawMesh(selectedModel->entity->pickingSphere, defaultShader);
            }
        }

        // Draw selected Skybox
        if (g_CurrentSkyboxIndex != 0)
        {
            if (Skyboxes->find(g_CurrentSkyboxIndex)->second->directory.length() > 0)
            {
                Shader *skyboxShader = sCache->getShader("skybox");
                skyboxShader->useProgram();
                skyboxShader->setUniform4fv("view", glm::mat4(glm::mat3(viewMatrix))); // remove translation from the view matrix
                renderer->drawSkybox(Skyboxes->find(g_CurrentSkyboxIndex)->second);
            }
        }

        // Draw GUI
        gui.newFrame();
        gui.performanceInfoOverlay(renderer, Info); // TODO: switch to string rendering
        gui.makePanel(10.f, 10.f);
        gui.windowAndInputSettings(Input);
        gui.cameraSettings(camera);
        gui.groundSettings(g_CurrentGroundIndex,
                           Grounds,
                           g_GroundMaxResolution);
        gui.skyboxSettings(g_CurrentSkyboxIndex,
                           Skyboxes);
        gui.moduleSettings(g_CurrentModuleIndex,
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

    for (auto it = Modules->begin(); it != Modules->end(); it++)
        delete it->second;
    delete Modules;

    for (auto it = Grounds->begin(); it != Grounds->end(); it++)
        delete it->second;
    delete Grounds;

    for (auto it = Skyboxes->begin(); it != Skyboxes->end(); it++)
        delete it->second;
    delete Skyboxes;

    // TODO: remove | useless
    delete Editor->MeshGrid;
    delete Editor->MeshAxisDebug;
    delete Editor;

    delete sCache;
    delete renderer;
    delete camera;
}

void PrepareAxisDebug(Mesh *mesh)
{    
    mesh->Vertices.clear();

    Vertex vXa;
    vXa.position = glm::vec3(0.0f, 0.1f, 0.0f);
    mesh->Vertices.push_back(vXa);
    Vertex vXb;
    vXb.position = glm::vec3(2.0f, 0.1f, 0.0f);
    mesh->Vertices.push_back(vXb);

    Vertex vYa;
    vYa.position = glm::vec3(0.0f, 0.1f, 0.0f);
    mesh->Vertices.push_back(vYa);
    Vertex vYb;
    vYb.position = glm::vec3(0.0f, 2.0f, 0.0f);
    mesh->Vertices.push_back(vYb);

    Vertex vZa;
    vZa.position = glm::vec3(0.0f, 0.1f, 0.0f);
    mesh->Vertices.push_back(vZa);
    Vertex vZb;
    vZb.position = glm::vec3(0.0f, 0.1f, -2.0f);
    mesh->Vertices.push_back(vZb);
    
    glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO);
    glBufferSubData(GL_ARRAY_BUFFER,
                    0,
                    mesh->Vertices.size() * sizeof(Vertex),
                    &mesh->Vertices[0]);
}

void PushReferenceGridSubData(Mesh *mesh, uint32 resolution)
{
    uint32 vCount = resolution * 4 + 4;			   // 44
    float32 b = (float32)resolution / 2.0f + 1.0f; // 6
    float32 a = -b;								   // -6
    float32 xPos = -((float32)resolution / 2.0f);  // -5
    float32 zPos = xPos;						   // -5

    mesh->Vertices.clear();
    uint32 i = 0;
    while (i < vCount / 2) // z axis ->
    {
        Vertex v;
        if (i % 2 == 0)
        {
            v.position = glm::vec3(a, 0.0f, zPos);
        }
        else
        {
            v.position = glm::vec3(b, 0.0f, zPos);
            zPos += 1.0f;
        }

        mesh->Vertices.push_back(v);
        i++;
    }

    while (i < vCount) // x axis ->
    {
        Vertex v;
        if (i % 2 == 0)
        {
            v.position = glm::vec3(xPos, 0.0f, a);
        }
        else
        {
            v.position = glm::vec3(xPos, 0.0f, b);
            xPos += 1.0f;
        }

        mesh->Vertices.push_back(v);
        i++;
    }
        
    glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO);
    glBufferSubData(GL_ARRAY_BUFFER,
                    0,
                    mesh->Vertices.size() * sizeof(Vertex),
                    &mesh->Vertices[0]);
}

void PushMouseRaySubData(Mesh *mesh, glm::vec3 origin, glm::vec3 direction)
{
    glm::vec3 target = origin + (direction * 1.0f);

    mesh->Vertices.clear();
    Vertex v;
    v.position = glm::vec3(origin.x, origin.y, origin.z - 0.1f);
    mesh->Vertices.push_back(v);
    v.position = target;
    mesh->Vertices.push_back(v);

    glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO);
    glBufferSubData(GL_ARRAY_BUFFER,
                    0,
                    mesh->Vertices.size() * sizeof(Vertex),
                    &mesh->Vertices[0]);
}
