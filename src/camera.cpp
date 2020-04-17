#include "camera.h"

static void update_camera_vectors(camera_t *Camera);

namespace camera
{
    camera_t* Construct(
	float32 windowWidth, float32 windowHeight,
	glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
	glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f),
	float32 yaw = g_DefaultYawSetting,
	float32 pitch = g_DefaultPitchSetting,
	float32 speed = g_DefaultSpeedSetting,
	float32 sensitivity = g_DefaultSensitivitySetting,
	float32 fov = g_DefaultFovSetting)
    {
	camera_settings* Settings = new camera_settings;
	Settings->Yaw = yaw;
	Settings->Pitch = pitch;
	Settings->Speed = speed;
	Settings->Sensitivity = sensitivity;
	Settings->Fov = fov;

	glm::mat4 projection = glm::perspective(
	    glm::radians(fov),
	    windowWidth / windowHeight,
	    0.1f, 100.0f);

	camera_t* Camera = new camera_t;
	Camera->Position = position;
	Camera->Front = glm::vec3(0.0f, 0.0f, -1.0f);
	Camera->WorldUp = worldUp;
	Camera->Settings = Settings;
	Camera->ProjectionMatrix = projection;

	update_camera_vectors(Camera);
	return Camera;
    }

    void Delete(camera_t *Camera)
    {
        delete Camera->Settings;
        delete Camera;
    }

    glm::mat4 GetViewMatrix(camera_t *Camera)
    {
	return glm::lookAt(Camera->Position, Camera->Position + Camera->Front, Camera->Up);
    }

    void ProcessMovementDirectional(camera_t *Camera, camera_movement direction, float32 deltaTime)
    {
	float32 velocity = Camera->Settings->Speed * deltaTime;
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

    void ProcessMovementAngles(camera_t *Camera, float32 xoffset, float32 yoffset, bool constrainPitch = true)
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

	update_camera_vectors(Camera);
    }

    void ProcessMovementFov(camera_t *Camera, float32 yoffset)
    {
	if (Camera->Settings->Fov >= 1.0f && Camera->Settings->Fov <= 45.0f)
	    Camera->Settings->Fov -= yoffset;
	if (Camera->Settings->Fov <= 1.0f)
	    Camera->Settings->Fov = 1.0f;
	if (Camera->Settings->Fov >= 45.0f)
	    Camera->Settings->Fov = 45.0f;
    }
}

static void update_camera_vectors(camera_t *Camera)
{
// Calculates the front vector from the Camera's (updated) Euler Angles
    glm::vec3 front;
    front.x = cos(glm::radians(Camera->Settings->Yaw)) * cos(glm::radians(Camera->Settings->Pitch));
    front.y = sin(glm::radians(Camera->Settings->Pitch));
    front.z = sin(glm::radians(Camera->Settings->Yaw)) * cos(glm::radians(Camera->Settings->Pitch));
    Camera->Front = glm::normalize(front);
    // Also re-calculate the Right and Up vector
    Camera->Right = glm::normalize(glm::cross(Camera->Front, Camera->WorldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
    Camera->Up    = glm::normalize(glm::cross(Camera->Right, Camera->Front));
}
