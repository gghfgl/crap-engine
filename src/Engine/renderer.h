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
    void drawLines(Mesh *mesh, float32 width, shader_t *shader);
    void drawMesh(Mesh *mesh, shader_t *shader);
    void drawInstanceMesh(Mesh *mesh, shader_t *shader, uint32 count);
    void drawModel(Model *model, shader_t *shader);
    void drawInstanceModel(Model *model, shader_t *shader, uint32 count);
    void drawSkybox(Skybox *skybox, shader_t *shader);
    
    bool wireframe;
    RendererStat stats;
};
