#include "camera.h"

Camera::Camera(float32 windowWidth, float32 windowHeight,
               glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
               glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f),
               float32 yaw = g_DefaultYawSetting,
               float32 pitch = g_DefaultPitchSetting,
               float32 speed = g_DefaultSpeedSetting,
               float32 sensitivity = g_DefaultSensitivitySetting,
               float32 fov = g_DefaultFovSetting,
               float32 nearPlane = 0.1f, float32 farPlane = 100.0f)
{
    CameraSetting *settings = new CameraSetting;
    settings->yaw = yaw;
    settings->pitch = pitch;
    settings->speed = speed;
    settings->sensitivity = sensitivity;
    settings->fov = fov;

    glm::mat4 projection = glm::perspective(
        glm::radians(fov),
        windowWidth / windowHeight,
        nearPlane, farPlane);
			
    this->settings = settings;
    this->position = position;
    this->front = glm::vec3(0.0f, 0.0f, -1.0f);
    this->worldUp = worldUp;
    this->projectionMatrix = projection;

    this->update_camera_vectors();
}

Camera::~Camera()
{
    delete this->settings;
}

glm::mat4 Camera::getViewMatrix()
{
    return glm::lookAt(this->position, this->position + this->front, this->up);
}

void Camera::processMovementDirection(CameraDirection direction,
                                      float32 deltaTime,
                                      float32 acceleration = 1.0f)
{
    float32 velocity = this->settings->speed * acceleration * deltaTime;
    if (direction == FORWARD)
        this->position += this->front * velocity;
    if (direction == BACKWARD)
        this->position -= this->front * velocity;
    if (direction == LEFT)
        this->position -= this->right * velocity;
    if (direction == RIGHT)
        this->position += this->right * velocity;
    if (direction == UP)
        this->position += this->up * velocity;
    if (direction == DOWN)
        this->position -= this->up * velocity;
}

void Camera::processMovementAngles(float32 xoffset,
                                   float32 yoffset,
                                   bool constrainPitch = true)
{
    xoffset *= this->settings->sensitivity;
    yoffset *= this->settings->sensitivity;
    this->settings->yaw += xoffset;
    this->settings->pitch += yoffset;

    if (constrainPitch)
    {
        if (this->settings->pitch > 89.0f)
            this->settings->pitch = 89.0f;
        if (this->settings->pitch < -89.0f)
            this->settings->pitch = -89.0f;
    }

    update_camera_vectors();
}

void Camera::processMovementFov(float32 yoffset)
{
    if (this->settings->fov >= 1.0f && this->settings->fov <= 45.0f)
        this->settings->fov -= yoffset;
    if (this->settings->fov <= 1.0f)
        this->settings->fov = 1.0f;
    if (this->settings->fov >= 45.0f)
        this->settings->fov = 45.0f;
}

void Camera::update_camera_vectors()
{
    // Calculates the front vector from the Camera's (updated) Euler Angles
    glm::vec3 front;
    front.x = cos(glm::radians(this->settings->yaw)) * cos(glm::radians(this->settings->pitch));
    front.y = sin(glm::radians(this->settings->pitch));
    front.z = sin(glm::radians(this->settings->yaw)) * cos(glm::radians(this->settings->pitch));
    this->front = glm::normalize(front);
    // Also re-calculate the Right and Up vector
    this->right = glm::normalize(glm::cross(this->front, this->worldUp)); // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
    this->up = glm::normalize(glm::cross(this->right, this->front));
}
