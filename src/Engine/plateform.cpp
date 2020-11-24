#include "plateform.h"

void GLAPIENTRY debug_message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void keyboard_callback(GLFWwindow *window, int32 key, int32 scancode, int32 action, int32 mode);
void mouse_button_callback(GLFWwindow *window, int32 button, int32 action, int32 mods);
void cursor_position_callback(GLFWwindow *window, float64 xpos, float64 ypos);
void mouse_scroll_callback(GLFWwindow *window, float64 xoffset, float64 yoffset);

static KeyboardEvent *g_KEYBOARD_EVENT;
static MouseEvent *g_MOUSE_EVENT;

Plateform::Plateform(uint32 width, uint32 height, const char *title)
{
    this->window = new Window(width, height, title);
    this->info = new PlateformInfo();
    this->info->vendor = (const char *)glGetString(GL_VENDOR);
    this->info->graphicAPI = (const char *)glGetString(GL_RENDERER);
    this->info->versionAPI = (const char *)glGetString(GL_VERSION);

    this->bind_input();
}

Plateform::~Plateform()
{
    delete this->window;
    delete this->input;
    delete this->info;
}

void Plateform::bind_input()
{
    int32 width, height;

    glfwSetKeyCallback(this->window->context, keyboard_callback);
    glfwSetMouseButtonCallback(this->window->context, mouse_button_callback);
    glfwSetCursorPosCallback(this->window->context, cursor_position_callback);
    glfwSetScrollCallback(this->window->context, mouse_scroll_callback);
    glfwSetInputMode(this->window->context, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwGetWindowSize(this->window->context, &width, &height);

    g_KEYBOARD_EVENT = new KeyboardEvent;
    g_MOUSE_EVENT = new MouseEvent;
    g_MOUSE_EVENT->posX = 0.0f;
    g_MOUSE_EVENT->posY = 0.0f;
    g_MOUSE_EVENT->offsetX = 0.0f;
    g_MOUSE_EVENT->offsetY = 0.0f;
    g_MOUSE_EVENT->scrollOffsetX = 0.0f;
    g_MOUSE_EVENT->scrollOffsetY = 0.0f;
    g_MOUSE_EVENT->lastX = width / 2.0f;
    g_MOUSE_EVENT->lastY = height / 2.0f;
    g_MOUSE_EVENT->leftButton = false;
    g_MOUSE_EVENT->leftButtonFirstClick = true;
    g_MOUSE_EVENT->rightButton = false;
    g_MOUSE_EVENT->rightButtonFirstClick = true;

    this->input = new InputState;
    this->input->keyboard = g_KEYBOARD_EVENT;
    this->input->mouse = g_MOUSE_EVENT;
}

InputState::~InputState()
{
    delete this->keyboard;
    delete this->mouse;
}

void InputState::updateMouseOffsets()
{
    if (this->mouse->leftButtonFirstClick)
    {
        this->mouse->lastX = this->mouse->posX;
        this->mouse->lastY = this->mouse->posY;
        this->mouse->leftButtonFirstClick = false;
    }

    this->mouse->offsetX = (float32)(this->mouse->posX - this->mouse->lastX);
    this->mouse->offsetY = (float32)(this->mouse->lastY - this->mouse->posY);

    this->mouse->lastX = this->mouse->posX;
    this->mouse->lastY = this->mouse->posY;
}

float32 InputState::getMouseScrollOffsetY()
{
    float32 rValue = (float32)this->mouse->scrollOffsetY;
    this->mouse->scrollOffsetY = 0.0f;

    return rValue;
}

Window::Window(uint32 width, uint32 height, const char *title)
{
    // GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_SAMPLES, 4); // MSAA 4 samples

    GLFWwindow *window = glfwCreateWindow((int)width, (int)height, title, nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSwapInterval(1);

    // OpenGL
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        printf("EXITCODE:666 Failed to initialize GLAD");
        int exitcode = 666; // TODO: meh ???
        exit(exitcode);
    }

    glEnable(GL_DEPTH_TEST); // store z-values in depth/z-buffer
    glDepthFunc(GL_LESS);
    // glEnable(GL_CULL_FACE); // face culling
    // glCullFace(GL_FRONT); // face culling
    glEnable(GL_MULTISAMPLE); // MSAA enable TODO: on the fly setting
    // glEnable(GL_BLEND);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // glEnable(GL_STENCIL_TEST);
    // glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    // glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glEnable(GL_LINE_SMOOTH); // TODO: meh? ...
    glEnable(GL_DEBUG_OUTPUT); // Faster? // TODO: on the fly setting
    // glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(debug_message_callback, NULL); // TODO: on the fly setting

    this->m_width = width;
    this->m_height = height;
    this->m_title = title;
    this->m_vsync = true;
    this->debug = false;
    this->context = window;

    this->set_time();
}

Window::~Window()
{
    terminate_window();
    delete this->time;
    //delete Context; // TODO: ??
}

void Window::toggleVsync()
{
    this->m_vsync = !this->m_vsync;
    glfwSwapInterval(this->m_vsync);
}

void Window::swapBuffer()
{
    glfwSwapBuffers(this->context);
    glFinish();
}

void Window::pollEvents()
{
    glfwPollEvents();
}

void Window::updateTime()
{
    float currentFrame = (float32)glfwGetTime();
    this->time->deltaTime = currentFrame - this->time->lastFrame;
    this->time->lastFrame = currentFrame;    
}

void Window::set_time()
{
    this->time = new FrameTime;
    this->time->deltaTime = 0.0;
    this->time->lastFrame = 0.0;
}


void Window::terminate_window()
{
    glfwDestroyWindow(this->context);
    glfwTerminate();
}

// ========================================================

void framebuffer_size_callback(GLFWwindow*, int width, int height)
{
    glViewport(0, 0, width, height);
}
void keyboard_callback(GLFWwindow*, int32 key, int32, int32 action, int32)
{
    if (key >= 0 && key < 1024)
    {
        if (action == GLFW_PRESS)
        {
            g_KEYBOARD_EVENT->isPressed[key] = true;
            g_KEYBOARD_EVENT->isReleased[key] = false;
        }
        else if (action == GLFW_RELEASE)
        {
            g_KEYBOARD_EVENT->isPressed[key] = false;
            g_KEYBOARD_EVENT->isReleased[key] = true;
        }
    }
}

//void mouse_button_callback(GLFWwindow *window, int32 button, int32 action, int32 mods)
void mouse_button_callback(GLFWwindow*, int32 button, int32 action, int32)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        g_MOUSE_EVENT->leftButton = true;

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        g_MOUSE_EVENT->leftButton = false;
        g_MOUSE_EVENT->leftButtonFirstClick = true;
    }

    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
        g_MOUSE_EVENT->rightButton = true;

    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
    {
        g_MOUSE_EVENT->rightButton = false;
        g_MOUSE_EVENT->rightButtonFirstClick = true;
    }
}

//void cursor_position_callback(GLFWwindow *window, float64 xpos, float64 ypos)
void cursor_position_callback(GLFWwindow*, float64 xpos, float64 ypos)
{
    g_MOUSE_EVENT->posX = xpos;
    g_MOUSE_EVENT->posY = ypos;
}

//void mouse_scroll_callback(GLFWwindow *window, float64 xoffset, float64 yoffset)
void mouse_scroll_callback(GLFWwindow*, float64 xoffset, float64 yoffset)
{
    g_MOUSE_EVENT->scrollOffsetX = xoffset;
    g_MOUSE_EVENT->scrollOffsetY = yoffset;
}

// void APIENTRY debug_message_callback(GLenum source, GLenum type, GLuint id,
// 									 GLenum severity, GLsizei length,
// 									 const GLchar *msg, const void *data)
void APIENTRY debug_message_callback(GLenum source, GLenum type, GLuint id,
                                     GLenum severity, GLsizei,
                                     const GLchar *msg, const void*)
{
    if (severity != GL_DEBUG_SEVERITY_NOTIFICATION)
    {
        const char *_source;
        const char *_type;
        const char *_severity;

        switch (source)
        {
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

        switch (type)
        {
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

        switch (severity)
        {
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
}
