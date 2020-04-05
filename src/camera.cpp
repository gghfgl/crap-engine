#include "camera.h"

// Calculates the front vector from the Camera's (updated) Euler Angles
void update_camera_vectors(camera *Camera)
{
    // Calculate the new Front vector
    glm::vec3 front;
    front.x = cos(glm::radians(Camera->Settings->Yaw)) * cos(glm::radians(Camera->Settings->Pitch));
    front.y = sin(glm::radians(Camera->Settings->Pitch));
    front.z = sin(glm::radians(Camera->Settings->Yaw)) * cos(glm::radians(Camera->Settings->Pitch));
    Camera->Front = glm::normalize(front);
    // Also re-calculate the Right and Up vector
    Camera->Right = glm::normalize(glm::cross(Camera->Front, Camera->WorldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
    Camera->Up    = glm::normalize(glm::cross(Camera->Right, Camera->Front));
}

// construct with vectors
camera* CameraConstruct(
    float windowWidth, float windowHeight,
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
    float yaw = globalDefaultYawSetting,
    float pitch = globalDefaultPitchSetting,
    float speed = globalDefaultSpeedSetting,
    float sensitivity = globalDefaultSensitivitySetting,
    float fov = globalDefaultFovSetting)
{
    camera_settings* Settings = new camera_settings();
    Settings->Yaw = yaw;
    Settings->Pitch = pitch;
    Settings->Speed = speed;
    Settings->Sensitivity = sensitivity;
    Settings->Fov = fov;

    glm::mat4 projection = glm::perspective(
	glm::radians(fov),
        windowWidth / windowHeight,
	0.1f, 100.0f);
    
    camera* Camera = new camera();
    Camera->Position = position;
    Camera->WorldUp = up;
    Camera->Settings = Settings;
    Camera->Front = glm::vec3(0.0f, 0.0f, -1.0f);
    Camera->ProjectionMatrix = projection;

    update_camera_vectors(Camera);
    return Camera;
}

void CameraDelete(camera *Camera)
{
    delete Camera;
}

// Returns the view matrix calculated using Euler Angles and the LookAt Matrix
glm::mat4 CameraGetViewMatrix(camera *Camera)
{
    return glm::lookAt(Camera->Position, Camera->Position + Camera->Front, Camera->Up);
}

void CameraProcessKeyboard(camera *Camera, camera_movement direction, float deltaTime)
{
    float velocity = Camera->Settings->Speed * deltaTime;
    if (direction == FORWARD)
	Camera->Position += Camera->Front * velocity;
    if (direction == BACKWARD)
	Camera->Position -= Camera->Front * velocity;
    if (direction == LEFT)
	Camera->Position -= Camera->Right * velocity;
    if (direction == RIGHT)
	Camera->Position += Camera->Right * velocity;
    if (direction == UP)
	Camera->Position += Camera->Up * velocity;
    if (direction == DOWN)
	Camera->Position -= Camera->Up * velocity;
}

void CameraProcessMouseMovement(camera *Camera, float xoffset, float yoffset, bool constrainPitch = true)
{
    xoffset *= Camera->Settings->Sensitivity;
    yoffset *= Camera->Settings->Sensitivity;
    Camera->Settings->Yaw   += xoffset;
    Camera->Settings->Pitch += yoffset;

    if (constrainPitch)
   
    {
	if (Camera->Settings->Pitch > 89.0f)
	    Camera->Settings->Pitch = 89.0f;
	if (Camera->Settings->Pitch < -89.0f)
	    Camera->Settings->Pitch = -89.0f;
    }

    // Update Front, Right and Up Vectors using the updated Euler angles
    update_camera_vectors(Camera);
}

// Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
void CameraProcessMouseScroll(camera *Camera, float yoffset)
{
    if (Camera->Settings->Fov >= 1.0f && Camera->Settings->Fov <= 45.0f)
	Camera->Settings->Fov -= yoffset;
    if (Camera->Settings->Fov <= 1.0f)
	Camera->Settings->Fov = 1.0f;
    if (Camera->Settings->Fov >= 45.0f)
	Camera->Settings->Fov = 45.0f;
}

static void CameraSettingsCollapseHeader(camera *Camera)
{
    if (ImGui::CollapsingHeader("Camera settings", ImGuiTreeNodeFlags_DefaultOpen))
    {
    	ImGui::Text("yaw: %.2f", Camera->Settings->Yaw);
    	ImGui::Text("pitch: %.2f", Camera->Settings->Pitch);
    	ImGui::Text("speed: %.2f", Camera->Settings->Speed);
    	ImGui::Text("sensitivity: %.2f", Camera->Settings->Sensitivity);
    	ImGui::Text("fov: %.2f", Camera->Settings->Fov);
    	ImGui::Text("pos: %.2f, %.2f, %.2f",
		    Camera->Position.x,
		    Camera->Position.y,
		    Camera->Position.z);

    	ImVec2 bSize(100, 20);
    	ImGui::Button("Reset Default", bSize);
    	ImGui::SameLine();
    	ImGui::Button("Reset Front", bSize);
    	ImGui::SameLine();
    	ImGui::Button("Reset Up", bSize);
    	ImGui::Separator();
    }
}
