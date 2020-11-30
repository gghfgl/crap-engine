#include "editor.h"
#include "editor_gui.h"

void PrepareAxisDebug(Mesh *Mesh); // TODO: rename origin
void PushReferenceGridSubData(Mesh *Mesh, uint32 resolution);
void PushMouseRaySubData(Mesh *Mesh, glm::vec3 origin, glm::vec3 direction);

static uint32 g_CurrentGroundIndex = 0;
static int32 g_GroundResolution = 10;
static const uint32 g_GroundMaxResolution = 50;
//const char* g_GroundDefaultModelFile = "./assets/models/terrain/untitled.obj"; // TODO: read all this kind of stuff from a default config file

// TODO: Clean mess below
static uint32 g_HoveredEntity = 0;
static uint32 g_SelectedEntity = 0;
static uint32 g_DragEntity = 0;
const float32 g_PickingSphereRadius = 0.2f; // Used for draw sphere and ray intersection

const uint32 g_ReferenceGridResolution = 50;

void RunEditorMode(Window *Window, InputState *Input, PlateformInfo *Info)
{
    Camera *camera = new Camera((float32)Window->getWidth(), (float32)Window->getHeight(), glm::vec3(0.0f, 5.0f, 10.0f));
    Renderer *renderer = new Renderer();

    // TODO: parse a list in a dedicated header file // sure about pointer?
    ShaderCache *sCache = new ShaderCache();
    sCache->compileAndAddShader("./shaders/default.glsl", "default", camera->projectionMatrix);
    printf("compile shader='default'\n");
    sCache->compileAndAddShader("./shaders/instanced.glsl", "instanced", camera->projectionMatrix);
    printf("compile shader='instanced'\n");
    sCache->compileAndAddShader("./shaders/color.glsl", "color", camera->projectionMatrix);
    printf("compile shader='color'\n");
    sCache->compileAndAddShader("./shaders/skybox.glsl", "skybox", camera->projectionMatrix);
    printf("compile shader='skybox'\n");

    // =================================================
    // Grid
    std::vector<uint32> uEmpty;
    std::vector<Texture> tEmpty;
    std::vector<Vertex> vGrid(g_ReferenceGridResolution * 4 + 4, Vertex());
    Mesh *MeshGrid = new Mesh(vGrid, uEmpty, tEmpty);

    // Axis Debug
    std::vector<Vertex> vAxisDebug(6, Vertex());
    Mesh *MeshAxisDebug = new Mesh(vAxisDebug, uEmpty, tEmpty);

    // Ray
    std::vector<Vertex> vRay(2, Vertex());
    Mesh *MeshRay = new Mesh(vRay, uEmpty, tEmpty);

    // Entitys array
    std::map<uint32, Entity*> *SCENE = new std::map<uint32, Entity*>;

    // Skybox
    std::vector<std::string> faces{
        "./assets/skybox/test/right.jpg",
        "./assets/skybox/test/left.jpg",
        "./assets/skybox/test/top.jpg",
        "./assets/skybox/test/bottom.jpg",
        "./assets/skybox/test/front.jpg",
        "./assets/skybox/test/back.jpg"};
    Skybox *skybox = new Skybox(faces);

    // Ground map
    std::map<uint32, Ground*> *Grounds = new std::map<uint32, Ground*>;

    // =================================================
    editor_t *Editor = new editor_t;
    Editor->Active = true;
    Editor->GridResolution = 0;
    Editor->MeshGrid = MeshGrid;
    Editor->MeshAxisDebug = MeshAxisDebug;
    Editor->MeshRay = MeshRay;
    Editor->skybox = skybox;
    Editor->ShowSkybox = false;

    EditorGui gui = EditorGui(Window);

    // TODO: tmp wainting editor struct
    PrepareAxisDebug(Editor->MeshAxisDebug);
    PushReferenceGridSubData(Editor->MeshGrid, g_ReferenceGridResolution);
    // =============================

    while (Editor->Active)
    {
        Window->updateTime();

        // NOTE: INPUTS ======================================>
        Window->pollEvents();

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

        // NOTE: SIMULATE  ======================================>
        glm::vec3 rayWorld = MouseRayDirectionWorld((float32)Input->mouse->posX,
                                                    (float32)Input->mouse->posY,
                                                    Window->getWidth(),
                                                    Window->getHeight(),
                                                    camera->projectionMatrix,
                                                    camera->getViewMatrix());

        // ground slider
        if (g_CurrentGroundIndex != 0 && g_GroundResolution != (int)Grounds->find(g_CurrentGroundIndex)->second->resolution)
        {
            Grounds->find(g_CurrentGroundIndex)->second->isGenerated = false;
            //ground->resolution = g_GroundResolution;
        }
        
        // mouse ray intersection sphere selector objects
        for (auto it = SCENE->begin(); it != SCENE->end(); it++)
        {
            float32 rayIntersection = 0.0f;
            glm::vec3 spherePos = glm::vec3(
                it->second->position.x,
                it->second->position.y,
                it->second->position.z);

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

        // NOTE: RENDERING ======================================>
        renderer->resetStats();
        renderer->newContext();
        glm::mat4 viewMatrix = camera->getViewMatrix();

        Shader *colorShader = sCache->getShader("color");
        colorShader->useProgram();
        colorShader->setUniform4fv("view", viewMatrix);
        colorShader->setUniform4fv("model", glm::mat4(1.0f));

        // draw reference grid
        colorShader->setUniform4f("color", glm::vec4(0.360f, 0.360f, 0.360f, 1.0f));
        renderer->drawLines(Editor->MeshGrid, 1.0f);

        // draw axis debug
        colorShader->setUniform4f("color", glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
        renderer->drawLines(Editor->MeshAxisDebug, 2.0f);

        // draw mouse ray
        PushMouseRaySubData(Editor->MeshRay, camera->position, rayWorld);
        colorShader->setUniform4f("color", glm::vec4(1.0f, 0.8f, 0.0f, 1.0));
        renderer->drawLines(Editor->MeshRay, 1.0f);

        // draw ground
        if (g_CurrentGroundIndex != 0){
            if (Grounds->find(g_CurrentGroundIndex)->second->entity->model != nullptr)
            {
                if (!Grounds->find(g_CurrentGroundIndex)->second->isGenerated)
                {
                    Grounds->find(g_CurrentGroundIndex)->second->clearInstance();
                    Grounds->find(g_CurrentGroundIndex)->second->updateModelMatrices(g_GroundResolution);
                    Grounds->find(g_CurrentGroundIndex)->second->instanceBufferID = renderer->prepareInstance(Grounds->find(g_CurrentGroundIndex)->second->entity->model,
                                                                                                              Grounds->find(g_CurrentGroundIndex)->second->modelMatrices,
                                                                                                              Grounds->find(g_CurrentGroundIndex)->second->resolution * Grounds->find(g_CurrentGroundIndex)->second->resolution);
                    Grounds->find(g_CurrentGroundIndex)->second->isGenerated = true;
                }

                Shader *instancedShader = sCache->getShader("instanced");
                instancedShader->useProgram();
                instancedShader->setUniform4fv("view", viewMatrix);
                instancedShader->setUniform4fv("model", glm::mat4(1.0f));
                renderer->drawInstanceModel(Grounds->find(g_CurrentGroundIndex)->second->entity->model,
                                            instancedShader,
                                            Grounds->find(g_CurrentGroundIndex)->second->resolution * Grounds->find(g_CurrentGroundIndex)->second->resolution);
            }
        }
                  
        // draw objs
        Shader *defaultShader = sCache->getShader("default");
        defaultShader->useProgram();
        defaultShader->setUniform4fv("view", viewMatrix);
        defaultShader->setUniform4fv("model", glm::mat4(1.0f));
        for (auto it = SCENE->begin(); it != SCENE->end(); it++)
        {
            bool isSelected = false;
            if (g_HoveredEntity == it->first || g_SelectedEntity == it->first)
                isSelected = true;

            if (g_DragEntity == it->first)
                it->second->position = pIntersection;

            // Model
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, it->second->position);
            model = glm::scale(model, glm::vec3(it->second->scale));
            model = glm::rotate(model, glm::radians(it->second->rotate),
                                glm::vec3(0.0f, 1.0f, 0.0f));
            defaultShader->setUniform4fv("model", model);
            defaultShader->setUniform1ui("flip_color", isSelected);
            renderer->drawModel(it->second->model, defaultShader);

            // Picking sphere
            model = glm::mat4(1.0f);
            model = glm::translate(model, it->second->position);
            defaultShader->setUniform4fv("model", model);
            defaultShader->setUniform1ui("flip_color", isSelected);
            renderer->drawMesh(it->second->pickingSphere, defaultShader);
        }

        // =================== E.N.V.I.R.O.N.M.E.N.T ===================

        // Skybox
        if (Editor->skybox != NULL && Editor->ShowSkybox)
        {
            Shader *skyboxShader = sCache->getShader("skybox");
            skyboxShader->useProgram();
            skyboxShader->setUniform4fv("view", glm::mat4(glm::mat3(viewMatrix))); // remove translation from the view matrix
            renderer->drawSkybox(Editor->skybox);
        }

        // =================== G.U.I ===================

        // draw GUI
        gui.newFrame();
        gui.performanceInfoOverlay(renderer, Info); // TODO: switch to string rendering
        gui.makePanel(10.f, 10.f);
        gui.windowAndInputSettings(Input);
        gui.cameraSettings(camera);
        gui.groundSettings(g_GroundResolution,
                           g_GroundMaxResolution,
                           g_CurrentGroundIndex,
                           Grounds);
        // gui.environmentSettings(ground,
        //                         g_GroundResolution,
        //                         g_GroundMaxResolution,
        //                         &Editor->ShowSkybox);
        //gui.entitiesSettings(SCENE, &g_SelectedEntity, g_PickingSphereRadius);
        gui.endPanel();
        gui.draw();

        // TODO:  update memory pool
        //Renderer->MemoryArena->MaxUsed = 0;

        Window->swapBuffer();
    }

    for (auto it = SCENE->begin(); it != SCENE->end(); it++)
        delete it->second;
    delete SCENE;

    for (auto it = Grounds->begin(); it != Grounds->end(); it++)
        delete it->second;
    delete Grounds;

    // TODO: implement complete full delete Editor method
    delete Editor->MeshGrid;
    delete Editor->MeshAxisDebug;
    delete Editor->MeshRay;
    delete Editor->skybox;
    delete Editor;

    delete sCache;

    // TODO: delete gui?
    
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
