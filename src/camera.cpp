#include "camera.h"
#include <SDL2/SDL.h>

Camera::Camera(glm::vec3 position) noexcept
    : _position(position), _front(glm::vec3(0.0f, 0.0f, -1.0f)), _world_up(glm::vec3(0.0f, 1.0f, 0.0f)), _yaw(-90.0f), _pitch(0.0f), _movement_speed(3.0f), _mouse_sensitivity(0.08f), _zoom(45.0f)
{
    _update_camera_vectors();
}

glm::mat4 Camera::get_view_matrix() const noexcept
{
    return glm::lookAt(_position, _position + _front, _up);
}

const glm::vec3& Camera::get_position() const noexcept
{
    return _position;
}

const float& Camera::get_zoom() const noexcept
{
    return _zoom;
}

void Camera::process_keyboard(CameraMovement direction, float delta_time) noexcept
{
    float velocity = _movement_speed * delta_time;
    if (direction == CameraMovement::Forward)
        _position += _front * velocity;
    if (direction == CameraMovement::Backward)
        _position -= _front * velocity;
    if (direction == CameraMovement::Left)
        _position -= _right * velocity;
    if (direction == CameraMovement::Right)
        _position += _right * velocity;
    if (direction == CameraMovement::Up)
        _position += _world_up * velocity;
    if (direction == CameraMovement::Down)
        _position -= _world_up * velocity;
}

void Camera::process_mouse_movement(float x_offset, float y_offset) noexcept
{
    const Uint32 mousestate = SDL_GetMouseState(nullptr, nullptr);

    if (!(mousestate & SDL_BUTTON_RMASK)) {
        return;
    }

    x_offset *= _mouse_sensitivity;
    y_offset *= _mouse_sensitivity;

    _yaw += x_offset;
    _pitch -= y_offset;

    // Make sure that screen doesn't get flipped when pitch is out of bounds 
    if (_pitch > 89.0f)
        _pitch = 89.0f;
    if (_pitch < -89.0f)
        _pitch = -89.0f;
    
    // Update front, right and up vectors using the updated euler angles
    _update_camera_vectors();
}

void Camera::process_mouse_scroll(float y_offset) noexcept
{
    _zoom -= y_offset;
    if (_zoom < 1.0f)
        _zoom = 1.0f;
    if (_zoom > 45.0f)
        _zoom = 45.0f;
}

void Camera::_update_camera_vectors() noexcept
{
    glm::vec3 front;
    front.x = cos(glm::radians(_yaw)) * cos(glm::radians(_pitch));
    front.y = sin(glm::radians(_pitch));
    front.z = sin(glm::radians(_yaw)) * cos(glm::radians(_pitch));

    // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement
    _front = glm::normalize(front);
    _right = glm::normalize(glm::cross(_front, _world_up));
    _up = glm::normalize(glm::cross(_right, _front));
}