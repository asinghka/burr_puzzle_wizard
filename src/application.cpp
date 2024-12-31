#include <stdexcept>
#include <format>
#include <vector>
#include <fmt/os.h>
#include <fmt/core.h>
#include <glm/glm.hpp>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl2.h>
#include <imgui/imgui_impl_opengl3.h>

#include "application.h"

Application::Application(uint32_t width, uint32_t height) noexcept : _width(width), _height(height), _camera(glm::vec3(0.5, 0.5f, 3.0f))
{
    _create_window();
    _init_opengl();
    _init_imgui();
}

Application::~Application() noexcept
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    glDeleteVertexArrays(1, &_cube_vertex_array_object);
    glDeleteBuffers(1, &_cube_vertex_buffer_object);
    glDeleteProgram(_shader_program);
    SDL_DestroyWindow(_window);
    SDL_Quit();
}

void Application::_update_delta_time() noexcept
{
    uint32_t delta = SDL_GetTicks() - _last_tick;
    _delta_time = static_cast<float>(delta) / 1000.0f; 
    _last_tick = SDL_GetTicks();
}

void Application::_handle_sdl_events(SDL_Event& e, bool& running) noexcept
{
    while (SDL_PollEvent(&e))
    {
        ImGui_ImplSDL2_ProcessEvent(&e);

        if (e.type == SDL_QUIT)
            running = false;

        if (e.type == SDL_MOUSEMOTION) {
            _process_mouse_motion_input(e);
        }

        if (e.type == SDL_MOUSEWHEEL) {
            _process_mouse_scroll_input(e);
        }

        if (e.type == SDL_WINDOWEVENT) {
            if (e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                _width = e.window.data1;
                _height = e.window.data2;
            }
        }
    }
}

void Application::_new_gui_frame() noexcept
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    {
        ImGui::Begin("Debug");
        ImGui::Text("Application framerate (frame time): %.0f FPS\t(%.0f ms)", 1.0f / _delta_time, _delta_time * 1000);
        ImGui::End();
    }

    {
        ImGui::Begin("Control");

        for (size_t i = 0; i < _wizard.get_num_pieces(); i ++) {

            std::string s = std::to_string(i + 1) + ". Piece";
            
            if (ImGui::TreeNode(s.c_str())) {
                if (ImGui::Button(" +x ")) {
                    _wizard.move_piece(i, {1, 0, 0});
                }
                ImGui::SameLine();
                if (ImGui::Button(" -x ")) {
                    _wizard.move_piece(i, {-1, 0, 0});
                }
                if (ImGui::Button(" +y ")) {
                    _wizard.move_piece(i, {0, 1, 0});
                }
                ImGui::SameLine();
                if (ImGui::Button(" -y ")) {
                    _wizard.move_piece(i, {0, -1, 0});
                }
                if (ImGui::Button(" +z ")) {
                    _wizard.move_piece(i, {0, 0, 1});
                }
                ImGui::SameLine();
                if (ImGui::Button(" -z ")) {
                    _wizard.move_piece(i, {0, 0, -1});
                }

                ImGui::TreePop();
            }
        }
        
        ImGui::End();
    }
}

void Application::_draw_gui() const noexcept
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Application::_swap_buffers() const noexcept
{
    SDL_GL_SwapWindow(_window);
}

void Application::_process_key_input(bool& running) noexcept
{
    // Process keyboard input
    const Uint8* keystate = SDL_GetKeyboardState(nullptr);
    
    if (keystate[SDL_SCANCODE_W])
        _camera.process_keyboard(CameraMovement::Forward, _delta_time);
    if (keystate[SDL_SCANCODE_S])
        _camera.process_keyboard(CameraMovement::Backward, _delta_time);
    if (keystate[SDL_SCANCODE_A])
        _camera.process_keyboard(CameraMovement::Left, _delta_time);
    if (keystate[SDL_SCANCODE_D])
        _camera.process_keyboard(CameraMovement::Right, _delta_time);
    if (keystate[SDL_SCANCODE_Q] | keystate[SDL_SCANCODE_SPACE])
        _camera.process_keyboard(CameraMovement::Up, _delta_time);
    if (keystate[SDL_SCANCODE_E] | keystate[SDL_SCANCODE_LCTRL])
        _camera.process_keyboard(CameraMovement::Down, _delta_time);
    if (keystate[SDL_SCANCODE_F])
        _wizard.solve();
    if (keystate[SDL_SCANCODE_ESCAPE])
        running = false;
}

void Application::_process_mouse_motion_input(const SDL_Event& e) noexcept
{
    const Uint32 mousestate = SDL_GetMouseState(nullptr, nullptr);
    
    if (mousestate & SDL_BUTTON_RMASK) {
        SDL_ShowCursor(SDL_DISABLE);
        SDL_SetWindowGrab(_window, SDL_TRUE);
        SDL_SetRelativeMouseMode(SDL_TRUE);

        const float delta_x = static_cast<float>(e.motion.xrel);
        const float delta_y = static_cast<float>(e.motion.yrel);
        
        _camera.process_mouse_movement(delta_x, delta_y);
        
    } else {
        SDL_ShowCursor(SDL_ENABLE);
        SDL_SetWindowGrab(_window, SDL_FALSE);
        SDL_SetRelativeMouseMode(SDL_FALSE);
    }
}

void Application::_process_mouse_scroll_input(const SDL_Event& e) noexcept
{
    const float delta_scroll = static_cast<float>(e.wheel.y);
    _camera.process_mouse_scroll(delta_scroll);
}

void Application::_render() const noexcept
{
    glViewport(0, 0, _width, _height);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(_shader_program);

    glm::mat4 model = glm::mat4(1.0f);
    glm::vec3 camera_position = _camera.get_position();
    glm::mat4 proj = glm::perspective(glm::radians(_camera.get_zoom()), static_cast<float>(_width) / static_cast<float>(_height), 0.1f, 100.0f);
    glm::mat4 view = _camera.get_view_matrix();
    glm::vec3 color = {1.0f, 1.0f, 1.0f};

    glUniformMatrix4fv(glGetUniformLocation(_shader_program, "u_model"), 1, GL_FALSE, &model[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(_shader_program, "u_proj"), 1, GL_FALSE, &proj[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(_shader_program, "u_view"), 1, GL_FALSE, &view[0][0]);
    glUniform3fv(glGetUniformLocation(_shader_program, "u_color"), 1, &color[0]);

    // Render Grid
    glBindVertexArray(_grid_vertex_array_object);
    glDrawArrays(GL_LINES, 0, 36);
    glBindVertexArray(0);
    
    // Render Cubes
    glUniform3f(glGetUniformLocation(_shader_program, "u_view_position"), camera_position.x, camera_position.y, camera_position.z);
    glUniform4f(glGetUniformLocation(_shader_program, "u_color"), 1.0f, 1.0f, 1.0f, 1.0f);

    std::vector<std::vector<utils::int3>> unit_cube_positions = _wizard.get_all_unit_cube_global_positions();
    
    const float cube_scale = 1.0f / static_cast<float>(_wizard.get_dim());
    
    for (size_t piece = 0; piece < unit_cube_positions.size(); piece++) {
        color = _wizard.get_color(piece);
        
        for (const auto& cube_position : unit_cube_positions[piece]) {
            model = glm::mat4(1.0f);
            model = glm::scale(model, {cube_scale, cube_scale, cube_scale});
            model = glm::translate(model, static_cast<glm::vec3>(cube_position));
            glUniformMatrix4fv(glGetUniformLocation(_shader_program, "u_model"), 1, GL_FALSE, &model[0][0]);

            glUniform3fv(glGetUniformLocation(_shader_program, "u_color"), 1, &color[0]);
            
            glBindVertexArray(_cube_vertex_array_object);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            glBindVertexArray(0);
        }
    }
}

void Application::run() noexcept
{
    SDL_Event e;
    bool running = true;
    
    while (running)
    {
        _handle_sdl_events(e, running);
        _update_delta_time();
        _process_key_input(running);
        
        _new_gui_frame();
        _render();
        _draw_gui();
        
        _swap_buffers();
    }
}

void Application::init_wizard(const std::filesystem::path& filepath) noexcept
{
    _wizard.read_puzzle_from_file(filepath);
    _wizard.init_field();
    _wizard.init_start_node();

    _wizard.solve();
}

void Application::_init_imgui() const noexcept
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(_window, _context);
    ImGui_ImplOpenGL3_Init();
}

void Application::_create_window()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        throw std::runtime_error(std::format("{} failed: {}", "SDL_Init", SDL_GetError()));
    }
    
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES,8);
    
    _window = SDL_CreateWindow("Burr Puzzle Wizard", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, _width, _height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    if (!_window) {
        throw std::runtime_error(std::format("{} failed: {}", "SDL_CreateWindow", SDL_GetError()));
    }
    
    _context = SDL_GL_CreateContext(_window);

    if (!_context) {
        throw std::runtime_error(std::format("{} failed: {}", "SDL_GL_CreateContext", SDL_GetError()));
    }

    SDL_GL_MakeCurrent(_window, _context);
    SDL_GL_SetSwapInterval(1);
}

// Vertex Shader source code
auto vertex_shader_source = R"(
    #version 330 core
    layout(location = 0) in vec4 position;
    layout(location = 1) in vec4 normal;
    
    out vec4 frag_normal;
    out vec4 frag_position;

    uniform mat4 u_proj;
    uniform mat4 u_view;
    uniform mat4 u_model;

    void main() {
        gl_Position = u_proj * u_view * u_model * position;
        
        frag_normal = transpose(inverse(u_model)) * normal;
        frag_position = u_model * position;
    }
)";

// Fragment Shader source code
auto fragment_shader_source = R"(
    #version 330 core
    
    in vec4 frag_normal;
    in vec4 frag_position;

    uniform vec3 u_view_position;
    uniform vec3 u_color;

    out vec4 color;

    void main() {
        vec3 norm = normalize(frag_normal.xyz);
        vec3 light_color = vec3(1.0, 1.0, 1.0);
        vec3 light_direction = vec3(0.2, 1.0, 0.3);

        // ambient
        float ambient_strength = 0.3;
        vec3 ambient = ambient_strength * light_color;
        
        // diffuse
        float diff = max(dot(norm, light_direction), 0.0);
        vec3 diffuse = diff * light_color;

        // specular
        float specular_strength = 0.5;
        vec3 view_direction = normalize(u_view_position - frag_position.xyz);
        vec3 reflect_direction = reflect(-light_direction, norm);
        float spec = pow(max(dot(view_direction, reflect_direction), 0.0), 8);
        vec3 specular = specular_strength * spec * light_color;

        vec3 result = (ambient + diffuse + specular) * u_color;
        color = vec4(result, 1.0);
    }
)";

GLuint CompileShader(GLenum type, const GLchar* source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar info_log[512];
        glGetShaderInfoLog(shader, 512, nullptr, info_log);
        throw std::runtime_error(std::format("{} failed: {}", "Shader compilation", info_log));
    }

    return shader;
}

void Application::_init_opengl()
{
    if (glewInit() != GLEW_OK) {
        throw std::runtime_error("glewInit failed.");
    }

    auto version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    fmt::println("OpenGL Version: {}", version);

    // Cube Vertex Data
    const GLfloat cube_vertices[] = {
        0.0f,  0.0f,  0.0f,  0.0f,  0.0f, -1.0f,
        1.0f,  0.0f,  0.0f,  0.0f,  0.0f, -1.0f,
        1.0f,  1.0f,  0.0f,  0.0f,  0.0f, -1.0f,
        1.0f,  1.0f,  0.0f,  0.0f,  0.0f, -1.0f,
        0.0f,  1.0f,  0.0f,  0.0f,  0.0f, -1.0f,
        0.0f,  0.0f,  0.0f,  0.0f,  0.0f, -1.0f,

        0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
        1.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
        1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,
        1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,
        0.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,
        0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,

        0.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f,
        0.0f,  1.0f,  0.0f, -1.0f,  0.0f,  0.0f,
        0.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
        0.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
        0.0f,  0.0f,  1.0f, -1.0f,  0.0f,  0.0f,
        0.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f,

        1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f,
        1.0f,  1.0f,  0.0f,  1.0f,  0.0f,  0.0f,
        1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
        1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
        1.0f,  0.0f,  1.0f,  1.0f,  0.0f,  0.0f,
        1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f,

        0.0f,  0.0f,  0.0f,  0.0f, -1.0f,  0.0f,
        1.0f,  0.0f,  0.0f,  0.0f, -1.0f,  0.0f,
        1.0f,  0.0f,  1.0f,  0.0f, -1.0f,  0.0f,
        1.0f,  0.0f,  1.0f,  0.0f, -1.0f,  0.0f,
        0.0f,  0.0f,  1.0f,  0.0f, -1.0f,  0.0f,
        0.0f,  0.0f,  0.0f,  0.0f, -1.0f,  0.0f,

        0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
        1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
        1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,
        1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,
        0.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,
        0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f
    };

    // Create Vertex Array Object and Vertex Buffer Object
    glGenVertexArrays(1, &_cube_vertex_array_object);
    glGenBuffers(1, &_cube_vertex_buffer_object);

    // Bind and set vertex buffer and attributes
    glBindVertexArray(_cube_vertex_array_object);

    glBindBuffer(GL_ARRAY_BUFFER, _cube_vertex_buffer_object);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void*>(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Grid Vertex Data
    const GLfloat grid_vertices[] = {
        0.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,

        0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,

        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f,

        0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,

        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 1.0f,

        1.0f, 1.0f, 1.0f,
        0.0f, 1.0f, 1.0f,

        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 0.0f,

        1.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 1.0f,

        1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 0.0f,

        1.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,

        1.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 0.0f,

        0.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 1.0f
    };

    // Create Vertex Array Object and Vertex Buffer Object
    glGenVertexArrays(1, &_grid_vertex_array_object);
    glGenBuffers(1, &_grid_vertex_buffer_object);

    // Bind and set vertex buffer and attributes
    glBindVertexArray(_grid_vertex_array_object);

    glBindBuffer(GL_ARRAY_BUFFER, _grid_vertex_buffer_object);
    glBufferData(GL_ARRAY_BUFFER, sizeof(grid_vertices), grid_vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(0);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    // Compile shaders and create shader program
    GLuint vertex_shader = CompileShader(GL_VERTEX_SHADER, vertex_shader_source);
    GLuint fragment_shader = CompileShader(GL_FRAGMENT_SHADER, fragment_shader_source);

    _shader_program = glCreateProgram();
    glAttachShader(_shader_program, vertex_shader);
    glAttachShader(_shader_program, fragment_shader);
    glLinkProgram(_shader_program);

    // TODO: cleanup runtime error messages of this type
    GLint success;
    glGetProgramiv(_shader_program, GL_LINK_STATUS, &success);
    if (!success) {
        GLchar info_log[512];
        glGetProgramInfoLog(_shader_program, 512, nullptr, info_log);
        throw std::runtime_error(std::format("{} failed: {}", "glGetProgramiv", info_log));
    }

    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
}
