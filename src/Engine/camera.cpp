#include "camera.h"

Camera::Camera(glm::vec3 position, glm::vec3 lookAt, glm::vec3 worldUp,
               float32 fov, float32 pitch, float32 aspect, float32 nearPlane, float32 farPlane)
{
    this->position = position;
    this->lookAt = lookAt;
    this->worldUp = worldUp;
    this->frontVector = glm::vec3(0.0f, 0.0f, -1.0f);
    
    this->fov = fov;
    this->pitch = pitch;
    this->aspect = aspect;
    this->nearPlane = nearPlane;
    this->farPlane = farPlane;

    this->update_projection_matrix();
    this->update_normalize_vectors();
    this->update_view_matrix();
}

void Camera::SetCameraView(glm::vec3 position, glm::vec3 lookAt, glm::vec3 upVector)
{
    this->position = position;
    this->lookAt = lookAt;
    this->upVector = upVector;

    this->update_normalize_vectors();
    this->update_view_matrix();
}

void Camera::UpdatePositionFromDirection(CameraDirection direction, float32 deltaTime, float32 acceleration = 1.0f)
{
    float32 velocity = this->speed * acceleration * deltaTime;
    if (direction == CAMERA_FORWARD)
        this->position += this->frontVector * velocity;
    if (direction == CAMERA_BACKWARD)
        this->position -= this->frontVector * velocity;
    if (direction == CAMERA_LEFT)
        this->position -= this->rightVector * velocity;
    if (direction == CAMERA_RIGHT)
        this->position += this->rightVector * velocity;
    if (direction == CAMERA_UP)
        this->position += this->upVector * velocity;
    if (direction == CAMERA_DOWN)
        this->position -= this->upVector * velocity;

    this->update_view_matrix();
}

void Camera::UpdatePositionFromAngle(float32 xoffset, float32 yoffset, bool constrainPitch = true)
{
    xoffset *= this->sensitivity;
    yoffset *= this->sensitivity;
    this->yaw += xoffset;
    this->pitch += yoffset;

    if (constrainPitch)
    {
        if (this->pitch > 89.0f)
            this->pitch = 89.0f;
        if (this->pitch < -89.0f)
            this->pitch = -89.0f;
    }

    this->update_normalize_vectors();
    this->update_view_matrix();
}

// TODO: not working properly. Need to manager y rotation and 60degres ground angle?
void Camera::UpdateArcballFromAngle(float32 xoffset, float32 yoffset, bool constrainPitch = true)
{
    // Get the homogenous position of the camera and pivot point
    glm::vec4 position(this->position.x, this->position.y, this->position.z, 1);
    glm::vec4 pivot(this->lookAt.x, this->lookAt.y, this->lookAt.z, 1);

    // step 1 : Calculate the amount of rotation given the mouse movement.
    uint32 viewportWidth = 1440;
    uint32 viewportHeight = 900;
    float32 deltaAngleX = (2 * M_PI / viewportWidth); // a movement from left to right = 2*PI = 360 deg
    float32 deltaAngleY = (M_PI / viewportHeight);  // a movement from top to bottom = PI = 180 deg
    float32 xAngle = xoffset * deltaAngleX;
    float32 yAngle = yoffset * deltaAngleY;

    // DEBUG
    std::cout << "xAngle=" << xAngle << " yAngle=" << yAngle << std::endl;

    // // Extra step to handle the problem when the camera direction is the same as the up vector
    glm::vec3 viewDir = -glm::transpose(this->viewMatrix)[2];
    float cosAngle = dot(viewDir, this->upVector);
    if (cosAngle * sin(deltaAngleY) > 0.99f)
        deltaAngleY = 0;

    glm::vec3 rightVector = glm::transpose(this->viewMatrix)[0]; // change with the member one
    
    // step 2: Rotate the camera around the pivot point on the first axis.
    glm::mat4x4 rotationMatrixX(1.0f);
    rotationMatrixX = glm::rotate(rotationMatrixX, xAngle, this->worldUp);
    position = (rotationMatrixX * (position - pivot)) + pivot;

    // step 3: Rotate the camera around the pivot point on the second axis.
    glm::mat4x4 rotationMatrixY(1.0f);
    rotationMatrixY = glm::rotate(rotationMatrixY, yAngle, rightVector);
    glm::vec3 finalPosition = (rotationMatrixY * (position - pivot)) + pivot;

    // Update the camera view (we keep the same lookat and the same up vector)
    SetCameraView(finalPosition, this->lookAt, this->worldUp);
}

void Camera::UpdateProjectionFromFOV(float32 yoffset)
{
    if (this->fov >= 1.0f && this->fov <= 45.0f)
        this->fov -= yoffset;
    if (this->fov <= 1.0f)
        this->fov = 1.0f;
    if (this->fov >= 45.0f)
        this->fov = 45.0f;

    this->update_projection_matrix();
}

void Camera::update_view_matrix()
{
    this->viewMatrix =  glm::lookAt(this->position, this->lookAt + this->frontVector, this->upVector);
}

void Camera::update_projection_matrix()
{
    this->projectionMatrix = glm::perspective(glm::radians(this->fov), this->aspect, this->nearPlane, this->farPlane);
}

void Camera::update_normalize_vectors()
{
    // Calculates the front vector from the Camera's (updated) Euler Angles
    glm::vec3 front;
    front.x = cos(glm::radians(this->yaw)) * cos(glm::radians(this->pitch));
    front.y = sin(glm::radians(this->pitch));
    front.z = sin(glm::radians(this->yaw)) * cos(glm::radians(this->pitch));
    this->frontVector = glm::normalize(front);
    // Also re-calculate the Right and Up vector
    this->rightVector = glm::normalize(glm::cross(this->frontVector, this->worldUp)); // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
    this->upVector = glm::normalize(glm::cross(this->rightVector, this->frontVector));
}
