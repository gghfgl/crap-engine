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
    void DrawLines(GPUMesh *mesh, float32 width);
    void DrawMesh(GPUMesh *mesh, Shader *shader);
    void DrawInstanceMesh(GPUMesh *mesh, Shader *shader, uint32 count);
    void DrawModel(Model *model, Shader *shader);
    void DrawModelOutline(Model *model, Shader *shader);
    void DrawInstanceModel(Model *model, Shader *shader, uint32 count);
    void DrawSkybox(Skybox *skybox);
    void DrawBoundingBox(BoundingBox *boudingBox);

    // TODO: meh
    void PrepareRaySubData(GPUMesh *mesh, glm::vec3 origin, glm::vec3 direction);
    void PrepareReferenceGridSubData(GPUMesh *mesh, uint32 resolution);
    void PrepareOriginDebug(GPUMesh *mesh);

    
    bool wireframe;
    RendererStat stats;
};
