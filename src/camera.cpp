#include "camera.h"

// Calculates the front vector from the Camera's (updated) Euler Angles
void update_camera_vectors(camera *Camera)
{
    // Calculate the new Front vector
    glm::vec3 front;
    front.x = cos(glm::radians(Camera->Yaw)) * cos(glm::radians(Camera->Pitch));
    front.y = sin(glm::radians(Camera->Pitch));
    front.z = sin(glm::radians(Camera->Yaw)) * cos(glm::radians(Camera->Pitch));
    Camera->Front = glm::normalize(front);
    // Also re-calculate the Right and Up vector
    Camera->Right = glm::normalize(glm::cross(Camera->Front, Camera->WorldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
    Camera->Up    = glm::normalize(glm::cross(Camera->Right, Camera->Front));
}

// construct with vectors
camera* CameraConstruct(
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
    float yaw = YAW,
    float pitch = PITCH
    )
{
    camera* Result = new camera();
    Result->Position = position;
    Result->WorldUp = up;
    Result->Yaw = yaw;
    Result->Pitch = pitch;
    Result->Front = glm::vec3(0.0f, 0.0f, -1.0f);
    Result->MovementSpeed = SPEED;
    Result->MouseSensitivity = SENSITIVITY;
    Result->Fov = FOV;

    update_camera_vectors(Result);
    return Result;
}
    
// construct with scalar values
camera* CameraConstruct(
    float posX, float posY,float posZ,
    float upX, float upY, float upZ,
    float yaw, float pitch
    )
{
    camera* Result = new camera();	
    Result->Position = glm::vec3(posX,posY, posZ);
    Result->WorldUp = glm::vec3(upX, upY, upZ);
    Result->Yaw = yaw;
    Result->Pitch = pitch;
    Result->Front = glm::vec3(0.0f, 0.0f, -1.0f);
    Result->MovementSpeed = SPEED;
    Result->MouseSensitivity = SENSITIVITY;
    Result->Fov = FOV;

    update_camera_vectors(Result);
    return Result;
}

void DeleteCamera(camera *Camera)
{
    delete Camera;
}

// Returns the view matrix calculated using Euler Angles and the LookAt Matrix
glm::mat4 GetCameraViewMatrix(camera *Camera)
{
    return glm::lookAt(Camera->Position, Camera->Position + Camera->Front, Camera->Up);
}

void ProcessCameraKeyboard(camera *Camera, camera_movement direction, float deltaTime)
{
    float velocity = Camera->MovementSpeed * deltaTime;
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

void ProcessCameraMouseMovement(camera *Camera, float xoffset, float yoffset, bool constrainPitch = true)
{
    xoffset *= Camera->MouseSensitivity;
    yoffset *= Camera->MouseSensitivity;
    Camera->Yaw   += xoffset;
    Camera->Pitch += yoffset;

    if (constrainPitch)
   
    {
	if (Camera->Pitch > 89.0f)
	    Camera->Pitch = 89.0f;
	if (Camera->Pitch < -89.0f)
	    Camera->Pitch = -89.0f;
    }

    // Update Front, Right and Up Vectors using the updated Euler angles
    update_camera_vectors(Camera);
}

// Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
void ProcessMouseScroll(camera *Camera, float yoffset)
{
    if (Camera->Fov >= 1.0f && Camera->Fov <= 45.0f)
	Camera->Fov -= yoffset;
    if (Camera->Fov <= 1.0f)
	Camera->Fov = 1.0f;
    if (Camera->Fov >= 45.0f)
	Camera->Fov = 45.0f;
}
