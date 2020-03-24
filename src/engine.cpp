#include "engine.h"
#include "resource_manager.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

engine* engine_construct() {
    engine* Result = new engine();
    Result->GlobalState = ENGINE_ACTIVE;
    Result->Width = 0;
    Result->Height = 0;

    return Result;
}

void delete_engine(engine *Engine) {
    ResourceManager::Clear(); // TODO improve resource manager
    delete_input_state(Engine->InputState);
    delete_camera(Engine->Camera);
    delete_renderer(Engine->Renderer);
    delete Engine;
    glfwTerminate();
}

int init_engine_data(engine *Engine, unsigned int width, unsigned int height, int options) {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

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
    glEnable(GL_DEPTH_TEST);

    if (options & POLYGONE_MODE)
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    camera *Camera = camera_construct(glm::vec3(0.0f, 5.0f, 10.0f));
    input_state *InputState = input_state_construct(window, width, height);

    // TODO improve this crap
    ResourceManager::LoadShader("../shaders/default.vs", "../shaders/default.fs", nullptr, "default");
    Shader defaultShader = ResourceManager::GetShader("default")->Use();
    // view/projection transformations (perspective by default)
    glm::mat4 projection = glm::perspective(glm::radians(Camera->Zoom), (float)width / (float)height, 0.1f, 100.0f);
    defaultShader.SetMatrix4("projection", projection);

    Engine->Window = window;
    Engine->InputState = InputState;
    Engine->Camera = Camera;
    Engine->Width = width;
    Engine->Height = height;    
    Engine->Renderer = renderer_construct(ResourceManager::GetShader("default"));

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

    Engine->Renderer->shader->Use();
    glm::mat4 view = get_camera_view_matrix(Engine->Camera);
    Engine->Renderer->shader->SetMatrix4("view", view);
    glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
    Engine->Renderer->shader->SetMatrix4("model", model);
}

void stop_rendering(engine *Engine) {
    glfwSwapBuffers(Engine->Window);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}
