#pragma once

#include <GL/glew.h>
#include <SDL2/SDL.h>

#include "camera.h"
#include "burr_puzzle_wizard.h"

class Application final
{
public:
    explicit Application(uint32_t width = 1600, uint32_t height = 900) noexcept;
    ~Application() noexcept;
    
    void run() noexcept;
    void init_wizard(const std::filesystem::path& filepath) noexcept;
    
private:
    void _create_window();
    void _init_opengl();
    void _init_imgui() const noexcept;
    
    void _handle_sdl_events(SDL_Event& e, bool& running) noexcept;
    void _new_gui_frame() noexcept;
    void _draw_gui() const noexcept;
    void _swap_buffers() const noexcept;
    void _process_key_input(bool& running) noexcept;
    void _process_mouse_motion_input(const SDL_Event& e) noexcept;
    void _process_mouse_scroll_input(const SDL_Event& e) noexcept;

    void _render() const noexcept;
    
    void _update_delta_time() noexcept;
    
private:
    // TODO: use ptr instead?
    // TODO: templated class nested in non-templated class?
    BurrPuzzleWizard<48> _wizard;
    
    uint32_t _width;
    uint32_t _height;
    SDL_Window* _window;
    SDL_GLContext _context;

    float _delta_time = 0;
    uint32_t _last_tick = 0;

    Camera _camera;
    
    GLuint _shader_program;
    GLuint _cube_vertex_array_object;
    GLuint _cube_vertex_buffer_object;

    GLuint _grid_vertex_array_object;
    GLuint _grid_vertex_buffer_object;
};