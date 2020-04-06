#include "input.h"

void key_callback(GLFWwindow* window, int32 key, int32 scancode, int32 action, int32 mode);
void mouse_button_callback(GLFWwindow* window, int32 button, int32 action, int32 mods);
void cursor_position_callback(GLFWwindow* window, float64 xpos, float64 ypos);
void scroll_callback(GLFWwindow* window, float64 xoffset, float64 yoffset);

static input_state *_INPUT_STATE_DATA;

input_state* InputStateConstruct(GLFWwindow* Window) {
    glfwSetKeyCallback(Window, key_callback);
    glfwSetMouseButtonCallback(Window, mouse_button_callback);
    glfwSetCursorPosCallback(Window, cursor_position_callback);
    glfwSetScrollCallback(Window, scroll_callback);

    glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    int32 width, height;;
    glfwGetWindowSize(Window, &width, &height);

    _INPUT_STATE_DATA = new input_state();
    _INPUT_STATE_DATA->MousePosX = 0.0f;
    _INPUT_STATE_DATA->MousePosY = 0.0f;
    _INPUT_STATE_DATA->MouseOffsetX = 0.0f;
    _INPUT_STATE_DATA->MouseOffsetY = 0.0f;
    _INPUT_STATE_DATA->MouseScrollOffsetX = 0.0f;
    _INPUT_STATE_DATA->MouseScrollOffsetY = 0.0f;
    _INPUT_STATE_DATA->MouseLastX = width / 2.0f;
    _INPUT_STATE_DATA->MouseLastY = height / 2.0f;
    _INPUT_STATE_DATA->MouseLeftButton = false;
    _INPUT_STATE_DATA->MouseLeftButtonFirstClick = true;

    return _INPUT_STATE_DATA;
}

void InputStateDelete(input_state *InputState) {
    delete InputState ;
}

void InputStatePollEvents()
{
    glfwPollEvents();
    // if (Engine->GlobalState == ENGINE_TERMINATE) // TODO: good without??
    // 	glfwSetWindowShouldClose(Engine->Window, GL_TRUE);
}

void InputStateUpdateMouseOffset(input_state *InputState) {
    if (InputState->MouseLeftButtonFirstClick) {
	InputState->MouseLastX = InputState->MousePosX;
	InputState->MouseLastY = InputState->MousePosY;
	InputState->MouseLeftButtonFirstClick = false;
    }

    InputState->MouseOffsetX = (float32)(InputState->MousePosX - InputState->MouseLastX);
    InputState->MouseOffsetY = (float32)(InputState->MouseLastY - InputState->MousePosY);

    InputState->MouseLastX = InputState->MousePosX;
    InputState->MouseLastY = InputState->MousePosY;
}

static void InputStateSettingsCollapseHeader(input_state *InputState)
{
    if (ImGui::CollapsingHeader("Input settings", ImGuiTreeNodeFlags_DefaultOpen))
    {
    	ImGui::Text("mX/mY: %d / %d", (int32)InputState->MousePosX, (int32)InputState->MousePosY);
    	ImGui::Separator();
    }
}

void key_callback(GLFWwindow* window, int32 key, int32 scancode, int32 action, int32 mode) {
    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS)	    
            _INPUT_STATE_DATA->Keyboard[key] = true;
        else if (action == GLFW_RELEASE)
            _INPUT_STATE_DATA->Keyboard[key] = false;
    }
}

void mouse_button_callback(GLFWwindow* window, int32 button, int32 action, int32 mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	_INPUT_STATE_DATA->MouseLeftButton = true;

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
	_INPUT_STATE_DATA->MouseLeftButton = false;
        _INPUT_STATE_DATA->MouseLeftButtonFirstClick = true;
    }
}

void cursor_position_callback(GLFWwindow* window, float64 xpos, float64 ypos) {
    _INPUT_STATE_DATA->MousePosX = xpos;
    _INPUT_STATE_DATA->MousePosY = ypos;
}

void scroll_callback(GLFWwindow* window, float64 xoffset, float64 yoffset) {
    _INPUT_STATE_DATA->MouseScrollOffsetX = xoffset;
    _INPUT_STATE_DATA->MouseScrollOffsetY = yoffset;
}
