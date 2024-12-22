#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum class CameraMovement {
    Forward,
    Backward,
    Left,
    Right,
    Up,
    Down
};

class Camera final
{
public:
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f)) noexcept;

    [[nodiscard]] glm::mat4 get_view_matrix() const noexcept;
    [[nodiscard]] const glm::vec3& get_position() const noexcept;
    [[nodiscard]] const float& get_zoom() const noexcept;
        
    void process_keyboard(CameraMovement direction, float delta_time) noexcept;
    void process_mouse_movement(float x_offset, float y_offset) noexcept;
    void process_mouse_scroll(float y_offset) noexcept;

private:
    void _update_camera_vectors() noexcept;
    
private:
    glm::vec3 _position;
    glm::vec3 _front;
    glm::vec3 _up;
    glm::vec3 _right;
    glm::vec3 _world_up;

    float _yaw;
    float _pitch;

    float _movement_speed;
    float _mouse_sensitivity;
    float _zoom;
};

