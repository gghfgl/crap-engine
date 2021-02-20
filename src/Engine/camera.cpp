#include "camera.h"

Camera::Camera(glm::vec3 position, glm::vec3 lookAt, glm::vec3 worldUp,
               float32 fov, float32 aspect, float32 nearPlane, float32 farPlane)
{
    m_position = position;
    m_lookAt = lookAt;
    m_worldUp = worldUp;
    m_frontVector = glm::vec3(0.0f, 0.0f, -1.0f);
    
    m_fov = fov;
    m_aspect = aspect;
    m_nearPlane = nearPlane;
    m_farPlane = farPlane;

    update_projection_matrix();
    update_normalize_vectors();
    update_view_matrix();
}

void Camera::SetCameraView(glm::vec3 position, glm::vec3 lookAt, glm::vec3 upVector)
{
    m_position = position;
    m_lookAt = lookAt;
    m_upVector = upVector;

    update_normalize_vectors();
    update_view_matrix();
}

void Camera::UpdatePositionFromDirection(CameraDirection direction, float32 deltaTime, float32 acceleration = 1.0f)
{
    float32 velocity = m_speed * acceleration * deltaTime;
    if (direction == CAMERA_FORWARD)
        m_position += m_frontVector * velocity;
    if (direction == CAMERA_BACKWARD)
        m_position -= m_frontVector * velocity;
    if (direction == CAMERA_LEFT)
        m_position -= m_rightVector * velocity;
    if (direction == CAMERA_RIGHT)
        m_position += m_rightVector * velocity;
    if (direction == CAMERA_UP)
        m_position += m_upVector * velocity;
    if (direction == CAMERA_DOWN)
        m_position -= m_upVector * velocity;

    update_view_matrix();
}

void Camera::UpdatePositionFromAngle(float32 xoffset, float32 yoffset, bool constrainPitch = true)
{
    xoffset *= m_sensitivity;
    yoffset *= m_sensitivity;
    m_yaw += xoffset;
    m_pitch += yoffset;

    if (constrainPitch)
    {
        if (m_pitch > 89.0f)
            m_pitch = 89.0f;
        if (m_pitch < -89.0f)
            m_pitch = -89.0f;
    }

    update_normalize_vectors();
    update_view_matrix();
}

// TODO: not working properly. Need to manager y rotation and 60degres ground angle?
void Camera::UpdateArcballFromAngle(float32 xoffset, float32 yoffset, bool constrainPitch = true)
{
    // Get the homogenous position of the camera and pivot point
    glm::vec4 position(m_position.x, m_position.y, m_position.z, 1);
    glm::vec4 pivot(m_lookAt.x, m_lookAt.y, m_lookAt.z, 1);

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
    glm::vec3 viewDir = -glm::transpose(m_viewMatrix)[2];
    float cosAngle = dot(viewDir, m_upVector);
    if (cosAngle * sin(deltaAngleY) > 0.99f)
        deltaAngleY = 0;

    glm::vec3 rightVector = glm::transpose(m_viewMatrix)[0]; // change with the member one
    
    // step 2: Rotate the camera around the pivot point on the first axis.
    glm::mat4x4 rotationMatrixX(1.0f);
    rotationMatrixX = glm::rotate(rotationMatrixX, xAngle, m_worldUp);
    position = (rotationMatrixX * (position - pivot)) + pivot;

    // step 3: Rotate the camera around the pivot point on the second axis.
    glm::mat4x4 rotationMatrixY(1.0f);
    rotationMatrixY = glm::rotate(rotationMatrixY, yAngle, rightVector);
    glm::vec3 finalPosition = (rotationMatrixY * (position - pivot)) + pivot;

    // Update the camera view (we keep the same lookat and the same up vector)
    SetCameraView(finalPosition, m_lookAt, m_worldUp);
}

void Camera::UpdateProjectionFromFOV(float32 yoffset)
{
    if (m_fov >= 1.0f && m_fov <= 45.0f)
        m_fov -= yoffset;
    if (m_fov <= 1.0f)
        m_fov = 1.0f;
    if (m_fov >= 45.0f)
        m_fov = 45.0f;

    update_projection_matrix();
}

void Camera::update_view_matrix()
{
    m_viewMatrix =  glm::lookAt(m_position, m_lookAt + m_frontVector, m_upVector);
}

void Camera::update_projection_matrix()
{
    m_projectionMatrix = glm::perspective(glm::radians(m_fov), m_aspect, m_nearPlane, m_farPlane);
}

void Camera::update_normalize_vectors()
{
    // Calculates the front vector from the Camera's (updated) Euler Angles
    glm::vec3 front;
    front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    front.y = sin(glm::radians(m_pitch));
    front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    m_frontVector = glm::normalize(front);
    // Also re-calculate the Right and Up vector
    m_rightVector = glm::normalize(glm::cross(m_frontVector, m_worldUp)); // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
    m_upVector = glm::normalize(glm::cross(m_rightVector, m_frontVector));
}
