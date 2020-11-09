#include <windows.h>

#include "plateform.h"

void GLAPIENTRY debug_message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void keyboard_callback(GLFWwindow *window, int32 key, int32 scancode, int32 action, int32 mode);
void mouse_button_callback(GLFWwindow *window, int32 button, int32 action, int32 mods);
void cursor_position_callback(GLFWwindow *window, float64 xpos, float64 ypos);
void mouse_scroll_callback(GLFWwindow *window, float64 xoffset, float64 yoffset);

static KeyboardEvent *g_KEYBOARD_EVENT;
static MouseEvent *g_MOUSE_EVENT;

Plateform::Plateform(uint32 t_width, uint32 t_height, const char *t_title)
{    
    Window = new WindowWrapper(t_width, t_height, t_title);

    RenderInfoAPI renderInfoAPI;
	renderInfoAPI.vendor = (const char *)glGetString(GL_VENDOR);
    renderInfoAPI.renderer = (const char *)glGetString(GL_RENDERER);
    renderInfoAPI.version = (const char *)glGetString(GL_VERSION);
    RenderAPI = renderInfoAPI;

    bind_input();
}

Plateform::~Plateform()
{
    delete Window;
    delete Input;
}

void Plateform::bind_input()
{
    int32 t_width, t_height;

    glfwSetKeyCallback(Window->Context, keyboard_callback);
    glfwSetMouseButtonCallback(Window->Context, mouse_button_callback);
    glfwSetCursorPosCallback(Window->Context, cursor_position_callback);
    glfwSetScrollCallback(Window->Context, mouse_scroll_callback);
    glfwSetInputMode(Window->Context, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwGetWindowSize(Window->Context, &t_width, &t_height);

    g_KEYBOARD_EVENT = new KeyboardEvent;
    g_MOUSE_EVENT = new MouseEvent;
    g_MOUSE_EVENT->posX = 0.0f;
    g_MOUSE_EVENT->posY = 0.0f;
    g_MOUSE_EVENT->offsetX = 0.0f;
    g_MOUSE_EVENT->offsetY = 0.0f;
    g_MOUSE_EVENT->scrollOffsetX = 0.0f;
    g_MOUSE_EVENT->scrollOffsetY = 0.0f;
    g_MOUSE_EVENT->lastX = t_width / 2.0f;
    g_MOUSE_EVENT->lastY = t_height / 2.0f;
    g_MOUSE_EVENT->leftButton = false;
    g_MOUSE_EVENT->leftButtonFirstClick = true;
    g_MOUSE_EVENT->rightButton = false;
    g_MOUSE_EVENT->rightButtonFirstClick = true;

    Input = new InputState;
    Input->KeyboardEvent = g_KEYBOARD_EVENT;
    Input->MouseEvent = g_MOUSE_EVENT;
}

InputState::~InputState()
{
    delete KeyboardEvent;
    delete MouseEvent;
}

void InputState::updateMouseOffsets()
{
    if (MouseEvent->leftButtonFirstClick)
    {
        MouseEvent->lastX = MouseEvent->posX;
        MouseEvent->lastY = MouseEvent->posY;
        MouseEvent->leftButtonFirstClick = false;
    }

    MouseEvent->offsetX = (float32)(MouseEvent->posX - MouseEvent->lastX);
    MouseEvent->offsetY = (float32)(MouseEvent->lastY - MouseEvent->posY);

    MouseEvent->lastX = MouseEvent->posX;
    MouseEvent->lastY = MouseEvent->posY;
}

float32 InputState::getMouseScrollOffsetY()
{
    float32 rValue = (float32)MouseEvent->scrollOffsetY;
    MouseEvent->scrollOffsetY = 0.0f;

    return rValue;
}

WindowWrapper::WindowWrapper(uint32 t_width, uint32 t_height, const char *t_title)
{
    // GLFW
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_SAMPLES, 4); // MSAA 4 samples

	GLFWwindow *window = glfwCreateWindow(t_width, t_height, t_title, nullptr, nullptr);
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

    m_width = t_width;
    m_height = t_height;
    m_title = t_title;
    m_vsync = true;
    debug = false;
    Context = window;

    set_time();
}

WindowWrapper::~WindowWrapper()
{
    terminate_window();
    delete Time;
    //delete Context; // TODO: ??
}

void WindowWrapper::toggleVsync()
{
    m_vsync = !m_vsync;
    glfwSwapInterval(m_vsync);
}

void WindowWrapper::swapBuffer()
{
	glfwSwapBuffers(Context);
	glFinish();
}

void WindowWrapper::pollEvents()
{
    glfwPollEvents();
}

void WindowWrapper::updateTime()
{
    LARGE_INTEGER currentPerfCount;
    QueryPerformanceCounter(&currentPerfCount);
    float32 counterElapsed = (float32)(currentPerfCount.QuadPart - Time->lastPerfCount);
    float32 msPerFrame = (1000 * counterElapsed) / (float32)Time->perfCountFrequency;
    int32 fps = (int32)(Time->perfCountFrequency / counterElapsed);

    uint64 currentCycleCount = __rdtsc();
    int64 cyclesElapsed = currentCycleCount - Time->lastCycleCount;
    int32 MCPF = (int32)(cyclesElapsed / (1000 * 1000));

    // float currentFrame = (float32)glfwGetTime();
    // Time->deltaTime = currentFrame - Time->msPerFrame; // s
    // Time->msPerFrame = currentFrame;
    
    Time->msPerFrame = msPerFrame;			// ms
    Time->deltaTime = msPerFrame / 1000.0f; // s
    Time->lastPerfCount = currentPerfCount.QuadPart;
    Time->fps = fps;
    Time->megaCyclePerFrame = MCPF;
    Time->lastCycleCount = currentCycleCount;
}

void WindowWrapper::set_time()
{
    LARGE_INTEGER perfCountFrequencyResult;
    QueryPerformanceFrequency(&perfCountFrequencyResult);
    LARGE_INTEGER lastPerfCount;
    QueryPerformanceCounter(&lastPerfCount);
    uint64 lastCycleCount = __rdtsc();

    Time = new FrameTime;
    Time->perfCountFrequency = perfCountFrequencyResult.QuadPart;
    Time->lastPerfCount = lastPerfCount.QuadPart;
    Time->lastCycleCount = lastCycleCount;

    // Time->deltaTime = 0.0; // s
    // Time->lastFrame = 0.0;
}


void WindowWrapper::terminate_window()
{
	glfwDestroyWindow(Context);
	glfwTerminate();
}

// ========================================================

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
	glViewport(0, 0, width, height);
}
void keyboard_callback(GLFWwindow *window, int32 key, int32 scancode, int32 action, int32 mode)
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

void mouse_button_callback(GLFWwindow *window, int32 button, int32 action, int32 mods)
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

void cursor_position_callback(GLFWwindow *window, float64 xpos, float64 ypos)
{
    g_MOUSE_EVENT->posX = xpos;
    g_MOUSE_EVENT->posY = ypos;
}

void mouse_scroll_callback(GLFWwindow *window, float64 xoffset, float64 yoffset)
{
    g_MOUSE_EVENT->scrollOffsetX = xoffset;
    g_MOUSE_EVENT->scrollOffsetY = yoffset;
}

void APIENTRY debug_message_callback(GLenum source, GLenum type, GLuint id,
									 GLenum severity, GLsizei length,
									 const GLchar *msg, const void *data)
{
	if (severity != GL_DEBUG_SEVERITY_NOTIFICATION)
	{
		char *_source;
		char *_type;
		char *_severity;

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
