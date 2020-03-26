#include "IMGUI/imgui.h"
#include "IMGUI/imgui_impl_glfw.h"
#include "IMGUI/imgui_impl_opengl3.h"

#include "engine.h"
#include "resource_manager.h"

/* NOTE: should implement
   - depth test / z-fighting
   - stencil test outline object
   - face culling
   - white / blanc texture
   - blending
   - framebuffer ?
   - mipmap ?
   - cubemap / skybox / reflect ?
   - geometry shader ?
   - instancing ?
   - MSAA anti aliasing ?
   - light system / PBR?
   - load models
*/

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

engine* engine_construct() {
    engine* Result = new engine();
    Result->GlobalState = ENGINE_ACTIVE;
    Result->OverlayState = OVERLAY_DEBUG;
    Result->Width = 0;
    Result->Height = 0;

    return Result;
}

void delete_imgui() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void delete_engine(engine *Engine) {
    delete_imgui();
    clear_resources();
    delete_input_state(Engine->InputState);
    delete_camera(Engine->Camera);
    delete_renderer(Engine->Renderer);
    delete Engine;

    glfwTerminate();
}

void init_imgui(GLFWwindow* window) {
    const char* glsl_version = "#version 150"; 
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
}

int init_engine_data(engine *Engine, unsigned int width, unsigned int height, int options) {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    //glfwWindowHint(GLFW_SAMPLES, 4); // TODO: Options -> MSAA 4 sample
    glfwSwapInterval(1); // TODO VSync

    GLFWwindow* window = glfwCreateWindow(width, height, "CrapEngine", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // NOTE: OpenGL configuration
    //glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    //glEnable(GL_CULL_FACE);
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_DEPTH_TEST); // store z-values in depth/z-buffer
    glEnable(GL_CULL_FACE); // TODO: Options -> face culling
    glCullFace(GL_FRONT); // TODO: Options -> face culling
    //glEnable(GL_MULTISAMPLE); // TODO: Options -> MSAA enable

    if (options & POLYGONE_MODE)
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    
    camera *Camera = camera_construct(glm::vec3(0.0f, 5.0f, 10.0f));
    input_state *InputState = input_state_construct(window, width, height);

    load_shader("../shaders/default.vs", "../shaders/default.fs", nullptr, "default");
    shader *DefaultShader = get_shader("default");
    unsigned int uniformBlockIndexDefault = glGetUniformBlockIndex(DefaultShader->ID, "Matrices");  
    glUniformBlockBinding(DefaultShader->ID, uniformBlockIndexDefault, 0);

    glGenBuffers(1, &Engine->UBO);
    glBindBuffer(GL_UNIFORM_BUFFER, Engine->UBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, Engine->UBO, 0, sizeof(glm::mat4));

    glm::mat4 projection = glm::perspective(glm::radians(Camera->Zoom), (float)width / (float)height, 0.1f, 100.0f);
    glBindBuffer(GL_UNIFORM_BUFFER, Engine->UBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    Engine->Window = window;
    Engine->InputState = InputState;
    Engine->Camera = Camera;
    Engine->Width = width;
    Engine->Height = height;    
    Engine->Renderer = renderer_construct(get_shader("default"));
    
    return 0;
}

void engine_update(engine *Engine, float deltaTime) {
    glfwPollEvents();
    if (Engine->GlobalState == ENGINE_TERMINATE)
	glfwSetWindowShouldClose(Engine->Window, GL_TRUE);
}

void start_rendering(engine *Engine) {
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 view = get_camera_view_matrix(Engine->Camera);
    use_shader(Engine->Renderer->Shader);
    shader_set_uniform4fv(Engine->Renderer->Shader, "view", view);
    
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
    shader_set_uniform4fv(Engine->Renderer->Shader, "model", model);
}

void stop_rendering(engine *Engine) {
    glfwSwapBuffers(Engine->Window);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void display_debug_overlay(engine *Engine, float deltaTime) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    const float DISTANCE = 10.0f;
    static int corner = 0;
    ImGuiIO& io = ImGui::GetIO();
    if (corner != -1)
    {
	ImVec2 window_pos = ImVec2((corner & 1) ? io.DisplaySize.x - DISTANCE : DISTANCE, (corner & 2) ? io.DisplaySize.y - DISTANCE : DISTANCE);
	ImVec2 window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
	ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
    }

    ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
    if (ImGui::Begin("Debug overlay", NULL, (corner != -1 ? ImGuiWindowFlags_NoMove : 0) | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav))
    {
	ImGui::Text("Debug overlay");
	ImGui::Separator();
	ImGui::Text((const char*)glGetString(GL_RENDERER));
	ImGui::Text((const char*)glGetString(GL_VERSION));
	ImGui::Separator();
	if (ImGui::IsMousePosValid())
	    ImGui::Text("Mouse Position: (%.1f,%.1f)", io.MousePos.x, io.MousePos.y);
	else
	    ImGui::Text("Mouse Position: <invalid>");
	ImGui::Text("CubeCount = %d", Engine->Renderer->Stats.CubeCount);
	ImGui::Text("DrawCount = %d", Engine->Renderer->Stats.DrawCount);
	ImGui::Text("Frame = %.3f ms", deltaTime);
	ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());    
}

// float enforcing_framerate_60fps(float currentFrame) {
//     float deltaTime = (float)(std::clock() - currentFrame);
//     unsigned int waitTime = (unsigned int)(FRAME_MIN_OFFSET_MS - deltaTime);

//     if (waitTime > 0.0f) {
// 	Sleep(waitTime);
//     }

//     deltaTime = (float)(std::clock() - currentFrame);
//     return deltaTime;
// }
