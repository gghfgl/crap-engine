#include <windows.h>

void GLAPIENTRY debug_message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void keyboard_callback(GLFWwindow *window, int32 key, int32 scancode, int32 action, int32 mode);
void mouse_button_callback(GLFWwindow *window, int32 button, int32 action, int32 mods);
void cursor_position_callback(GLFWwindow *window, float64 xpos, float64 ypos);
void mouse_scroll_callback(GLFWwindow *window, float64 xoffset, float64 yoffset);

static keyboard_e *KEYBOARD_EVENTS;
static mouse_e *MOUSE_EVENTS;

void win32_init_window(uint32 width, uint32 height, const char *windowTitle, window_t *Window)
{
    // GLFW
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_SAMPLES, 4); // MSAA 4 samples

	GLFWwindow *window = glfwCreateWindow(width, height, windowTitle, nullptr, nullptr);
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
    
    // Engine
	render_API_info APIinfo;
	APIinfo.Vendor = (const char *)glGetString(GL_VENDOR);
	APIinfo.Renderer = (const char *)glGetString(GL_RENDERER);
	APIinfo.Version = (const char *)glGetString(GL_VERSION);

    Window->Win32Window = window;
    Window->Width = width;
    Window->Height = height;
    Window->RenderAPIinfo = APIinfo;
    Window->WindowTitle = windowTitle;
    Window->Vsync = true;
}

void win32_init_time(frame_time *Time)
{
    LARGE_INTEGER perfCountFrequencyResult;
    QueryPerformanceFrequency(&perfCountFrequencyResult);
    LARGE_INTEGER lastPerfCount;
    QueryPerformanceCounter(&lastPerfCount);
    uint64 lastCycleCount = __rdtsc();

    Time->PerfCountFrequency = perfCountFrequencyResult.QuadPart;
    Time->LastPerfCount = lastPerfCount.QuadPart;
    Time->LastCycleCount = lastCycleCount;    
}

void win32_terminate_window(window_t *Window)
{
	glfwDestroyWindow(Window->Win32Window);
	glfwTerminate();
}

void win32_update_time(frame_time *Time)
{
    LARGE_INTEGER currentPerfCount;
    QueryPerformanceCounter(&currentPerfCount);
    float32 counterElapsed = (float32)(currentPerfCount.QuadPart - Time->LastPerfCount);
    float32 msPerFrame = (1000 * counterElapsed) / (float32)Time->PerfCountFrequency;
    int32 fps = (int32)(Time->PerfCountFrequency / counterElapsed);

    uint64 currentCycleCount = __rdtsc();
    int64 cyclesElapsed = currentCycleCount - Time->LastCycleCount;
    int32 MCPF = (int32)(cyclesElapsed / (1000 * 1000));

    Time->MsPerFrame = msPerFrame;			// ms
    Time->DeltaTime = msPerFrame / 1000.0f; // s
    Time->LastPerfCount = currentPerfCount.QuadPart;
    Time->FPS = fps;
    Time->MegaCyclePerFrame = MCPF;
    Time->LastCycleCount = currentCycleCount;
}

void win32_toggle_vsync(window_t *Window)
{
    Window->Vsync = !Window->Vsync;
    glfwSwapInterval(Window->Vsync);
}

void win32_swap_buffer(window_t *Window)
{
	glfwSwapBuffers(Window->Win32Window);
	glFinish();
}

void win32_init_input(GLFWwindow *Window, input_t *Input)
{
    int32 width, height;

    glfwSetKeyCallback(Window, keyboard_callback);
    glfwSetMouseButtonCallback(Window, mouse_button_callback);
    glfwSetCursorPosCallback(Window, cursor_position_callback);
    glfwSetScrollCallback(Window, mouse_scroll_callback);
    glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwGetWindowSize(Window, &width, &height);

    KEYBOARD_EVENTS = new keyboard_e;
    MOUSE_EVENTS = new mouse_e;
    MOUSE_EVENTS->PosX = 0.0f;
    MOUSE_EVENTS->PosY = 0.0f;
    MOUSE_EVENTS->OffsetX = 0.0f;
    MOUSE_EVENTS->OffsetY = 0.0f;
    MOUSE_EVENTS->ScrollOffsetX = 0.0f;
    MOUSE_EVENTS->ScrollOffsetY = 0.0f;
    MOUSE_EVENTS->LastX = width / 2.0f;
    MOUSE_EVENTS->LastY = height / 2.0f;
    MOUSE_EVENTS->LeftButton = false;
    MOUSE_EVENTS->LeftButtonFirstClick = true;
    MOUSE_EVENTS->RightButton = false;
    MOUSE_EVENTS->RightButtonFirstClick = true;

    Input->KeyboardEvent = KEYBOARD_EVENTS;
    Input->MouseEvent = MOUSE_EVENTS;
}

void win32_poll_events()
{
    glfwPollEvents();
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
		    KEYBOARD_EVENTS->IsPressed[key] = true;
		    KEYBOARD_EVENTS->IsReleased[key] = false;
		}
		else if (action == GLFW_RELEASE)
		{
		    KEYBOARD_EVENTS->IsPressed[key] = false;
		    KEYBOARD_EVENTS->IsReleased[key] = true;
		}
	}
}

void mouse_button_callback(GLFWwindow *window, int32 button, int32 action, int32 mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	    MOUSE_EVENTS->LeftButton = true;

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
	{
	    MOUSE_EVENTS->LeftButton = false;
	    MOUSE_EVENTS->LeftButtonFirstClick = true;
	}

	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
	    MOUSE_EVENTS->RightButton = true;

	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
	{
	    MOUSE_EVENTS->RightButton = false;
	    MOUSE_EVENTS->RightButtonFirstClick = true;
	}
}

void cursor_position_callback(GLFWwindow *window, float64 xpos, float64 ypos)
{
    MOUSE_EVENTS->PosX = xpos;
    MOUSE_EVENTS->PosY = ypos;
}

void mouse_scroll_callback(GLFWwindow *window, float64 xoffset, float64 yoffset)
{
    MOUSE_EVENTS->ScrollOffsetX = xoffset;
    MOUSE_EVENTS->ScrollOffsetY = yoffset;
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
