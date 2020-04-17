#include "window.h"

// ===================== platform code =====================
void GLAPIENTRY debug_message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

static GLFWwindow* init_window(uint32 width, uint32 height, const char* windowTitle);
static render_API_info init_render_API();
static void swap_buffer_and_finish(window_t *Window);
static void terminate_window(window_t *Window);
// ========================================================

namespace window
{
    window_t* Construct(uint32 width, uint32 height, const char* windowTitle)
    {
        // ===================== platform code =====================
	GLFWwindow* window = init_window(width, height, windowTitle);
	render_API_info APIinfo = init_render_API();

	LARGE_INTEGER perfCountFrequencyResult;
	QueryPerformanceFrequency(&perfCountFrequencyResult);
	LARGE_INTEGER lastPerfCount;
	QueryPerformanceCounter(&lastPerfCount);
	uint64 lastCycleCount = __rdtsc();
        // ========================================================
    
        window_time *Time = new window_time;
	Time->PerfCountFrequency = perfCountFrequencyResult.QuadPart;
	Time->LastPerfCount = lastPerfCount.QuadPart;
	Time->LastCycleCount = lastCycleCount;

	window_t *Window = new window_t;
	Window->PlatformWindow = window;
	Window->Width = width;
	Window->Height = height;
	Window->APIinfo = APIinfo;
	Window->WindowTitle = windowTitle;
	Window->Time = Time;
	Window->Vsync = true;

	return Window;
    }

    void UpdateTime(window_time *Time)
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

    void ToggleVsync(window_t *Window)
    {
	Window->Vsync = !Window->Vsync;
	glfwSwapInterval(Window->Vsync);
   }

    void ToggleDebugMode(window_t *Window)
    {
	    Window->DebugMode = !Window->DebugMode;
    }

    void SwapBuffer(window_t *Window)
    {
	swap_buffer_and_finish(Window);
    }

    void Delete(window_t *Window)
    {
        terminate_window(Window);
	delete Window->Time;
	delete Window;
    }
}

// ===================== platform code =====================
static GLFWwindow* init_window(uint32 width, uint32 height, const char* windowTitle)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    // glfwWindowHint(GLFW_SAMPLES, 4); // MSAA 4 samples

    GLFWwindow* window = glfwCreateWindow(width, height, windowTitle, nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSwapInterval(1);
    
    return window;
}

static void swap_buffer_and_finish(window_t *Window)
{
    glfwSwapBuffers(Window->PlatformWindow);
    glFinish();
}

static void terminate_window(window_t *Window)
{
    glfwDestroyWindow(Window->PlatformWindow);
    glfwTerminate();
}

static render_API_info init_render_API()
{
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
	// TODO: dont use STD::COUT!!!!
	std::cout << "EXITCODE:1234567890 Failed to initialize GLAD" << std::endl;
	int exitcode = 1234567890;
        exit(exitcode);
    }

    glEnable(GL_DEPTH_TEST); // store z-values in depth/z-buffer
    glDepthFunc(GL_LESS);
    //glEnable(GL_CULL_FACE); // face culling
    //glCullFace(GL_FRONT); // face culling
    // glEnable(GL_MULTISAMPLE); // MSAA enable TODO: on the fly setting

    //glEnable(GL_STENCIL_TEST);
    // glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    // glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    
    glEnable(GL_DEBUG_OUTPUT); // Faster? // TODO: on the fly setting
    //glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(debug_message_callback, NULL); // TODO: on the fly setting

    render_API_info APIinfo;
    APIinfo.GPUvendor = (const char*)glGetString(GL_RENDERER);
    APIinfo.Version = (const char*)glGetString(GL_VERSION);

    return APIinfo;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void APIENTRY debug_message_callback(GLenum source, GLenum type, GLuint id,
				     GLenum severity, GLsizei length,
				     const GLchar *msg, const void *data)
{
    if (severity != GL_DEBUG_SEVERITY_NOTIFICATION)
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
}
// ========================================================
