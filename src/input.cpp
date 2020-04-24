#include "input.h"

// ===================== platform code =====================
void keyboard_callback(GLFWwindow* window, int32 key, int32 scancode, int32 action, int32 mode);
void mouse_button_callback(GLFWwindow* window, int32 button, int32 action, int32 mods);
void cursor_position_callback(GLFWwindow* window, float64 xpos, float64 ypos);
void mouse_scroll_callback(GLFWwindow* window, float64 xoffset, float64 yoffset);
// ========================================================

namespace input
{
    static keyboard_e *KEYBOARD_EVENTS;
    static mouse_e *MOUSE_EVENTS;

    input_t* Construct(GLFWwindow* Window)
    {
	int32 width, height;

	// ===================== platform code =====================
	glfwSetKeyCallback(Window, keyboard_callback);
	glfwSetMouseButtonCallback(Window, mouse_button_callback);
	glfwSetCursorPosCallback(Window, cursor_position_callback);
	glfwSetScrollCallback(Window, mouse_scroll_callback);
	glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	glfwGetWindowSize(Window, &width, &height);
	// ========================================================

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
    
	input_t *InputState = new input_t;
	InputState->KeyboardEvent = KEYBOARD_EVENTS;
	InputState->MouseEvent = MOUSE_EVENTS;

	return InputState;
    }

    void Delete(input_t *InputState)
    {
        delete InputState->KeyboardEvent;
	delete InputState->MouseEvent;
	delete InputState;
    }

    void PollEvents()
    {
	// ===================== platform code =====================
	glfwPollEvents();
	// ========================================================
    }

    void UpdateMouseOffsets(mouse_e *MouseEvent)
    {
	if (MouseEvent->LeftButtonFirstClick) {
	    MouseEvent->LastX = MouseEvent->PosX;
	    MouseEvent->LastY = MouseEvent->PosY;
	    MouseEvent->LeftButtonFirstClick = false;
	}

	MouseEvent->OffsetX = (float32)(MouseEvent->PosX - MouseEvent->LastX);
	MouseEvent->OffsetY = (float32)(MouseEvent->LastY - MouseEvent->PosY);

	MouseEvent->LastX = MouseEvent->PosX;
	MouseEvent->LastY = MouseEvent->PosY;
    }

    float32 GetMouseScrollOffsetY(mouse_e *MouseEvent)
    {
	float32 rValue =  (float32)MouseEvent->ScrollOffsetY;
	MouseEvent->ScrollOffsetY = 0.0f;

	return rValue;
    }
    
    glm::vec3 MouseRayDirectionWorld(float32 mouseX,float32 mouseY,
				     int width, int height,
				     glm::mat4 projectionMatrix,
				     glm::mat4 viewMatrix)
    {
	// transform to NDC
	float32 mx = (2.0f * mouseX) / (float32)width - 1.0f;
	float32 my = (2.0f * mouseY) / (float32)height - 1.0f;
	glm::vec2 ndcRay = glm::vec2(mx, -my);

	// Clip space
	glm::vec4 clipRay = glm::vec4(ndcRay.x, ndcRay.y, -1.0, 1.0);

	// Eye space
	glm::vec4 eyeRay = inverse(projectionMatrix) * clipRay;
	eyeRay = glm::vec4(eyeRay.x, eyeRay.y, -1.0, 0.0);

	// World space
	glm::vec4 worldRay = inverse(viewMatrix) * eyeRay;
	glm::vec3 mouseRay = glm::vec3(worldRay.x, worldRay.y, worldRay.z);

	return normalize(mouseRay);
    }
}

// ===================== platform code =====================
void keyboard_callback(GLFWwindow* window, int32 key, int32 scancode, int32 action, int32 mode)
{
    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS)
	{
	    input::KEYBOARD_EVENTS->IsPressed[key] = true;
	    input::KEYBOARD_EVENTS->IsReleased[key] = false;
	}
        else if (action == GLFW_RELEASE)
	{
	    input::KEYBOARD_EVENTS->IsPressed[key] = false;
	    input::KEYBOARD_EVENTS->IsReleased[key] = true;
	}
    }
}

void mouse_button_callback(GLFWwindow* window, int32 button, int32 action, int32 mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	input::MOUSE_EVENTS->LeftButton = true;

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
	input::MOUSE_EVENTS->LeftButton = false;
	input::MOUSE_EVENTS->LeftButtonFirstClick = true;
    }

    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
	input::MOUSE_EVENTS->RightButton = true;

    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
	input::MOUSE_EVENTS->RightButton = false;
	input::MOUSE_EVENTS->RightButtonFirstClick = true;
    }
}

void cursor_position_callback(GLFWwindow* window, float64 xpos, float64 ypos)
{
    input::MOUSE_EVENTS->PosX = xpos;
    input::MOUSE_EVENTS->PosY = ypos;
}

void mouse_scroll_callback(GLFWwindow* window, float64 xoffset, float64 yoffset)
{
    input::MOUSE_EVENTS->ScrollOffsetX = xoffset;
    input::MOUSE_EVENTS->ScrollOffsetY = yoffset;
}
// ========================================================
