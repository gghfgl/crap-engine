#include "IMGUI/imgui.h"
#include "IMGUI/imgui_impl_glfw.h"
#include "IMGUI/imgui_impl_opengl3.h"

#include "engine.h"
#include "resource_manager.h"

void GLAPIENTRY DebugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

void init_imgui(GLFWwindow* window)
{
    const char* glsl_version = "#version 150"; 
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
}

engine* EngineConstructAndInit(unsigned int width, unsigned int height, int options=NO_FLAG)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    //glfwWindowHint(GLFW_SAMPLES, 4); // MSAA 4 samples

    GLFWwindow* window = glfwCreateWindow(width, height, "CrapEngine", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        // return -1;
    }

    if (options & VSYNC)
	glfwSwapInterval(1); // VSync
    else
	glfwSwapInterval(0); // VSync

    glEnable(GL_DEPTH_TEST); // store z-values in depth/z-buffer
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE); // face culling
    glCullFace(GL_FRONT); // face culling
    //glEnable(GL_MULTISAMPLE); // MSAA enable
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    
    //glEnable(GL_DEBUG_OUTPUT); // Faster? // TODO
    //glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    //glDebugMessageCallback(DebugMessageCallback, NULL); // TODO: !!!

    engine *Engine = new engine();

    if (options & DEBUG_MODE)
        Engine->DebugMode = true;    
    if (options & POLYGONE_MODE)
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    
    camera *Camera = CameraConstruct(glm::vec3(0.0f, 5.0f, 10.0f));
    input_state *InputState = InputStateConstruct(window, width, height);

    LoadShader("../shaders/default.vs", "../shaders/default.fs", nullptr, "default");
    shader *DefaultShader = GetShader("default");
    LoadShader("../shaders/default.vs", "../shaders/stencil.fs", nullptr, "stencil");
    shader *StencilShader = GetShader("stencil");

    unsigned int uniformBlockIndexDefault = glGetUniformBlockIndex(DefaultShader->ID, "Matrices");  
    glUniformBlockBinding(DefaultShader->ID, uniformBlockIndexDefault, 0);
    glGenBuffers(1, &Engine->UBO);
    glBindBuffer(GL_UNIFORM_BUFFER, Engine->UBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, Engine->UBO, 0, sizeof(glm::mat4));

    glm::mat4 projection = glm::perspective(
	glm::radians(Camera->Fov),
	(float)width / (float)height,
	0.1f, 100.0f);
    glBindBuffer(GL_UNIFORM_BUFFER, Engine->UBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    Engine->GlobalState = ENGINE_ACTIVE;
    Engine->Window = window;
    Engine->Time = new engine_time();
    Engine->InputState = InputState;
    Engine->Camera = Camera;
    Engine->ProjMatrix = projection;
    Engine->Width = width;
    Engine->Height = height;    
    Engine->Renderer = RendererConstruct(DefaultShader);
    Engine->Renderer->Stencil = StencilShader;

    init_imgui(window);
    return Engine;
}

void delete_imgui()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void DeleteEngine(engine *Engine)
{
    delete_imgui();
    ClearResources();
    DeleteInputState(Engine->InputState);
    DeleteCamera(Engine->Camera);
    CleanAndDeleteRenderer(Engine->Renderer);
    delete Engine->Time;
    delete Engine;

    glfwTerminate();
}

void UpdateDeltaTimeAndFPS(engine_time *Time)
{
   const double currentTime = glfwGetTime();
   Time->DeltaTime = currentTime - Time->LastFrameTime;
   Time->LastFrameTime = currentTime;
   Time->NextFPS++;

   if(currentTime - Time->LastFrameTimeFPS > 1.0)
   {
      Time->LastFrameTimeFPS = currentTime;
      Time->FPS = Time->NextFPS;
      Time->NextFPS = 0;
   }
}

void EnginePollEvents(engine *Engine)
{
    glfwPollEvents();
    if (Engine->GlobalState == ENGINE_TERMINATE)
	glfwSetWindowShouldClose(Engine->Window, GL_TRUE);
}

void StartRendering(renderer *Renderer, camera *Camera)
{
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glm::mat4 view = GetCameraViewMatrix(Camera);
    glm::mat4 model = glm::mat4(1.0f);
    //model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));

    UseShader(Renderer->Shader);
    ShaderSetUniform4fv(Renderer->Shader, "view", view);    
    ShaderSetUniform4fv(Renderer->Shader, "model", model);

    glStencilMask(0x00); // dont update the stencil buffer
}

void StartImGuiRendering()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void RenderImGui()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());    
}

void StartStencilRendering(renderer *Renderer, camera *Camera)
{
    // TODO: assume that ClearColor was made before
    float scale = 1.1f;
    glm::mat4 view = GetCameraViewMatrix(Camera);
    glm::mat4 model = glm::mat4(1.0f);
    //model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
    //model = glm::scale(model, glm::vec3(scale, scale, scale));

    UseShader(Renderer->Stencil);
    ShaderSetUniform4fv(Renderer->Stencil, "view", view);    
    ShaderSetUniform4fv(Renderer->Stencil, "model", model);

    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilMask(0x00);
    glDisable(GL_DEPTH_TEST);
}

void StopRenderingStencil()
{
    glStencilMask(0xFF);
    glDisable(GL_STENCIL_TEST);

}

void SwapBufferAndFinish(GLFWwindow *Window)
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST); // TODO: do some check?
    glfwSwapBuffers(Window);
    glFinish();
}

void DrawDebugOverlay(engine *Engine)
{
    // const float DISTANCE = 10.0f;
    // static int corner = 0;
    ImGuiIO& io = ImGui::GetIO();
    // if (corner != -1)   
    // {
    // 	ImVec2 window_pos = ImVec2(
    // 	    (corner & 1) ? io.DisplaySize.x - DISTANCE : DISTANCE,
    // 	    (corner & 2) ? io.DisplaySize.y - DISTANCE : DISTANCE);
    // 	ImVec2 window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
    // 	ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
    // }

    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - 210, 10));
    ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
    if (ImGui::Begin("Debug overlay", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav))   
    {
	ImGui::Text("Debug overlay");
	ImGui::Separator();
	ImGui::Text((const char*)glGetString(GL_RENDERER));
	ImGui::Text((const char*)glGetString(GL_VERSION));
	ImGui::Separator();
	ImGui::Text("dt: %.3f ms", Engine->Time->DeltaTime);
	ImGui::SameLine();
	ImGui::Text("fps: %d", Engine->Time->FPS);
	ImGui::End();
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void APIENTRY DebugMessageCallback(GLenum source, GLenum type, GLuint id,
                            GLenum severity, GLsizei length,
                            const GLchar *msg, const void *data)
{
    char* _source;
    char* _type;
    char* _severity;

    switch (source) {
        case GL_DEBUG_SOURCE_API:
        _source = "API";
        break;

        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        _source = "WINDOW SYSTEM";
        break;

        case GL_DEBUG_SOURCE_SHADER_COMPILER:
        _source = "SHADER COMPILER";
        break;

        case GL_DEBUG_SOURCE_THIRD_PARTY:
        _source = "THIRD PARTY";
        break;

        case GL_DEBUG_SOURCE_APPLICATION:
        _source = "APPLICATION";
        break;

        case GL_DEBUG_SOURCE_OTHER:
        _source = "UNKNOWN";
        break;

        default:
        _source = "UNKNOWN";
        break;
    }

    switch (type) {
        case GL_DEBUG_TYPE_ERROR:
        _type = "ERROR";
        break;

        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        _type = "DEPRECATED BEHAVIOR";
        break;

        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        _type = "UDEFINED BEHAVIOR";
        break;

        case GL_DEBUG_TYPE_PORTABILITY:
        _type = "PORTABILITY";
        break;

        case GL_DEBUG_TYPE_PERFORMANCE:
        _type = "PERFORMANCE";
        break;

        case GL_DEBUG_TYPE_OTHER:
        _type = "OTHER";
        break;

        case GL_DEBUG_TYPE_MARKER:
        _type = "MARKER";
        break;

        default:
        _type = "UNKNOWN";
        break;
    }

    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:
        _severity = "HIGH";
        break;

        case GL_DEBUG_SEVERITY_MEDIUM:
        _severity = "MEDIUM";
        break;

        case GL_DEBUG_SEVERITY_LOW:
        _severity = "LOW";
        break;

        case GL_DEBUG_SEVERITY_NOTIFICATION:
        _severity = "NOTIFICATION";
        break;

        default:
        _severity = "UNKNOWN";
        break;
    }

    printf("%d: %s of %s severity, raised from %s: %s\n",
            id, _type, _severity, _source, msg);
}
