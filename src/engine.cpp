#include "engine.h"

void GLAPIENTRY debug_message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

void init_imgui(GLFWwindow* window)
{
    const char* glsl_version = "#version 450"; 
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    //ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
}

void delete_imgui()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void WrapImGuiNewFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void WrapImGuiRender()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());    
}

engine* EngineConstruct(uint32 width, uint32 height, int32 options=NO_FLAG)
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

    if (options & VSYNC) // TODO: on the fly setting
	glfwSwapInterval(1); // VSync
    else
	glfwSwapInterval(0); // VSync

    glEnable(GL_DEPTH_TEST); // store z-values in depth/z-buffer
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE); // face culling
    glCullFace(GL_FRONT); // face culling
    //glEnable(GL_MULTISAMPLE); // MSAA enable TODO: on the fly setting
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    
    //glEnable(GL_DEBUG_OUTPUT); // Faster? // TODO: on the fly setting
    //glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    //glDebugMessageCallback(debug_message_callback, NULL); // TODO: on the fly setting

    if (options & POLYGONE_MODE) // TODO: on the fly setting
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    LARGE_INTEGER perfCountFrequencyResult;
    QueryPerformanceFrequency(&perfCountFrequencyResult);
    LARGE_INTEGER lastPerfCount;
    QueryPerformanceCounter(&lastPerfCount);
    uint64 lastCycleCount = __rdtsc();

    engine_time *Time = new engine_time;
    Time->PerfCountFrequency = perfCountFrequencyResult.QuadPart;
    Time->LastPerfCount = lastPerfCount.QuadPart;
    Time->LastCycleCount = lastCycleCount;

    engine *Engine = new engine();
    Engine->GlobalState = ENGINE_ACTIVE;
    Engine->GPUModel = (const char*)glGetString(GL_RENDERER);
    Engine->OpenglVersion = (const char*)glGetString(GL_VERSION);
    Engine->Window = window;
    Engine->Time = Time;

    init_imgui(window);
    return Engine;
}

void EngineDelete(engine *Engine)
{
    delete_imgui();
    delete Engine->Time;
    delete Engine;
    glfwTerminate();
}

void EngineGetWindowSize(engine *Engine,  uint32 width, uint32 height)
{
    glfwGetWindowSize(Engine->Window, &(int)width, &(int)height);
}

void EngineUpdateTime(engine_time *Time)
{
    LARGE_INTEGER currentPerfCount;
    QueryPerformanceCounter(&currentPerfCount);
    float32 counterElapsed = (float32)(currentPerfCount.QuadPart - Time->LastPerfCount);
    float32 msPerFrame = (1000 * counterElapsed) / (float32)Time->PerfCountFrequency;
    int32 fps = (int32)(Time->PerfCountFrequency / counterElapsed);

    uint64 currentCycleCount = __rdtsc();
    int64 cyclesElapsed = currentCycleCount - Time->LastCycleCount;
    int32 MCPF = (int32)(cyclesElapsed / (1000 * 1000));
    
    Time->MsPerFrame = msPerFrame;  // ms
    Time->DeltaTime = msPerFrame / 1000.0f; // s
    Time->LastPerfCount = currentPerfCount.QuadPart;
    Time->FPS = fps;
    Time->MegaCyclePerFrame = MCPF;
    Time->LastCycleCount = currentCycleCount;
}

void EngineShowOverlay(engine *Engine)
{
    ImGuiIO& io = ImGui::GetIO(); // TODO: move to engine widow get width
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - 210, 10));
    ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
    if (ImGui::Begin("Debug overlay", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav))   
    {
	ImGui::Text("Debug overlay");
	ImGui::Separator();
	ImGui::Text(Engine->GPUModel);
	ImGui::Text(Engine->OpenglVersion);
	ImGui::Separator();
	ImGui::Text("frame time: %.3fms", Engine->Time->MsPerFrame);
	ImGui::Text("frame per sec: %d", Engine->Time->FPS);
	ImGui::Text("Mcy per frame: %d", Engine->Time->MegaCyclePerFrame);
	ImGui::End();
    }
}

static void EngineSettingsCollapseHeader(engine *Engine, int width, int height)
{
    if (ImGui::CollapsingHeader("Engine settings", ImGuiTreeNodeFlags_DefaultOpen))
    {
	ImGui::Text("screen: %d x %d", width, height);
	ImGui::Text("VSYNC: ");
	ImGui::SameLine();
	if (Engine->Vsync)
	    ImGui::Button("on");
	else
	    ImGui::Button("off");
	ImGui::Text("Poly: ");
	ImGui::SameLine();
	if (Engine->PolyMode)
	    ImGui::Button("on");
	else
	    ImGui::Button("off");
	ImGui::Text("Debug: ");
	ImGui::SameLine();
	if (Engine->DebugMode)
	    ImGui::Button("on");
	else
	    ImGui::Button("off");
	ImGui::Separator();
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void APIENTRY debug_message_callback(GLenum source, GLenum type, GLuint id,
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
