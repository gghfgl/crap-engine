#pragma once

struct RendererStat
{
    uint32 drawCalls;
    int64 vertexCount;
};

struct Renderer
{
    Renderer();
    ~Renderer();
    void NewContext();
    void ResetStats();
    void ToggleWireframe();
    uint32 PrepareInstance(Model *model, glm::mat4 *modelMatrices, uint32 count);
    void DrawLines(Mesh *mesh, float32 width);
    void DrawMesh(Mesh *mesh, Shader *shader);
    void DrawInstanceMesh(Mesh *mesh, Shader *shader, uint32 count);
    void DrawModel(Model *model, Shader *shader);
    void DrawModelOutline(Model *model, Shader *shader);
    void DrawInstanceModel(Model *model, Shader *shader, uint32 count);
    void DrawSkybox(Skybox *skybox);

    // TODO: meh
    void PrepareRaySubData(Mesh *mesh, glm::vec3 origin, glm::vec3 direction);
    void PrepareReferenceGridSubData(Mesh *mesh, uint32 resolution);
    void PrepareOriginDebug(Mesh *mesh);

    
    bool wireframe;
    RendererStat stats;
};
