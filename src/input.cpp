#include "input.h"

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

input_state *_INPUT_STATE_DATA;

input_state* input_state_construct(GLFWwindow* Window, unsigned int width, unsigned int height) {
    glfwSetKeyCallback(Window, key_callback);
    glfwSetMouseButtonCallback(Window, mouse_button_callback);
    glfwSetCursorPosCallback(Window, cursor_position_callback);
    glfwSetScrollCallback(Window, scroll_callback);

    glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    _INPUT_STATE_DATA = new input_state();
    _INPUT_STATE_DATA->Window = Window;
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

void delete_input_state(input_state *InputState) {
    delete InputState ;
}

void update_mouse_offset(input_state *InputState) {
    if (InputState->MouseLeftButtonFirstClick) {
	InputState->MouseLastX = InputState->MousePosX;
	InputState->MouseLastY = InputState->MousePosY;
	InputState->MouseLeftButtonFirstClick = false;
    }

    InputState->MouseOffsetX = (float)(InputState->MousePosX - InputState->MouseLastX);
    InputState->MouseOffsetY = (float)(InputState->MouseLastY - InputState->MousePosY);

    InputState->MouseLastX = InputState->MousePosX;
    InputState->MouseLastY = InputState->MousePosY;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    
    if (key >= 0 && key < 1024)
    {
        if (action == GLFW_PRESS)	    
            _INPUT_STATE_DATA->Keyboard[key] = true;
        else if (action == GLFW_RELEASE)
            _INPUT_STATE_DATA->Keyboard[key] = false;
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	_INPUT_STATE_DATA->MouseLeftButton = true;

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
	_INPUT_STATE_DATA->MouseLeftButton = false;
        _INPUT_STATE_DATA->MouseLeftButtonFirstClick = true;
    }
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    _INPUT_STATE_DATA->MousePosX = xpos;
    _INPUT_STATE_DATA->MousePosY = ypos;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    _INPUT_STATE_DATA->MouseScrollOffsetX = xoffset;
    _INPUT_STATE_DATA->MouseScrollOffsetY = yoffset;
}
