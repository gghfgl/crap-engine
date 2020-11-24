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
    void newContext();
    void resetStats();
    void toggleWireframe();
    uint32 prepareInstance(Model *model, glm::mat4 *modelMatrices, uint32 count);
    void drawLines(Mesh *mesh, float32 width);
    void drawMesh(Mesh *mesh, Shader *shader);
    void drawInstanceMesh(Mesh *mesh, Shader *shader, uint32 count);
    void drawModel(Model *model, Shader *shader);
    void drawInstanceModel(Model *model, Shader *shader, uint32 count);
    void drawSkybox(Skybox *skybox);
    
    bool wireframe;
    RendererStat stats;
};
