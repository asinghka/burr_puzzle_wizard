#pragma once

#include <filesystem>
#include <sstream>
#include <fstream>
#include <queue>
#include <ranges>
#include <stack>
#include <unordered_set>

#include "node.h"
#include "piece.h"
#include "utils.h"

namespace std
{
    template<>
    struct hash<Node>
    {
        size_t operator()(const Node& key) const noexcept
        {
            std::vector<utils::int3> copy = key.get_key();
            std::vector<int> copy_int;

            for (auto int3 : copy) {
                for (size_t i = 0; i < 3; i++) {
                    copy_int.push_back(int3[i]);
                }
            }
            
            std::string string_data(copy_int.begin(), copy_int.end());
            return hash<std::string>()(string_data);
        }
    };

    template<>
    struct hash<std::vector<utils::int3>>
    {
        size_t operator()(const std::vector<utils::int3>& key) const noexcept
        {
            std::vector<utils::int3> copy = key;
            std::vector<int> copy_int;

            for (auto int3 : copy) {
                for (size_t i = 0; i < 3; i++) {
                    copy_int.push_back(int3[i]);
                }
            }
            
            std::string string_data(copy_int.begin(), copy_int.end());
            return hash<std::string>()(string_data);
        }
    };
}

template <size_t N>
class BurrPuzzleWizard final
{
public:
    BurrPuzzleWizard() = default;

    void read_puzzle_from_file(const std::filesystem::path& path) noexcept
    {
        std::bitset<N*N*N> temp_piece;
        std::vector<Piece<N>> temp_pieces;
        std::vector<utils::int3> temp_initial_positions;

        std::ifstream stream(path);

        if (stream.is_open()) {
            std::string line;

            while (getline(stream, line)) {

                // line[0] != '\r' for macOS
                if (!line.empty() && line[0] != '\r') {
                    std::vector<int> coordinates;

                    std::istringstream iss(line);
                    std::string word;
                    
                    if (line.at(0) == '#') {
                        while (iss >> word) {
                            if (word != "#") {
                                coordinates.push_back(stoi(word));
                            }
                        }

                        temp_initial_positions.push_back({coordinates[0], coordinates[1], coordinates[2]});
                    } else {
                        while (iss >> word) {
                            coordinates.push_back(stoi(word));
                        }

                        int index = utils::transform_index_3d_to_1d(coordinates[0], coordinates[1], coordinates[2], N);
                        temp_piece[index] = 1;
                    }
                } else {
                    temp_pieces.push_back(temp_piece);
                    temp_piece.reset();
                }
            }
        }

        _num_pieces = temp_pieces.size();
        _puzzle = temp_pieces;
        _initial_positions = temp_initial_positions;
        _positions = temp_initial_positions;

        stream.close();
    }

    void init_field() noexcept
    {
        for (size_t i = 0; i < _num_pieces; i++) {
            int index = utils::transform_index_3d_to_1d(_initial_positions[i], _dim);

            _field |= _puzzle[i].get_bitset(index);
        }
    }

    void init_start_node() noexcept
    {
        _start = Node(_initial_positions, N);
    }

    void move_piece(size_t index, utils::int3 direction) noexcept
    {
        if (_collides(index, direction))
            return;

        _field ^= _puzzle[index].get_bitset(_positions[index]);
        _positions[index] += direction;
        _field |= _puzzle[index].get_bitset(_positions[index]);
    }
    
    [[nodiscard]] std::vector<std::vector<utils::int3>> get_all_unit_cube_global_positions() const noexcept
    {
        std::vector<std::vector<utils::int3>> all_unit_cube_coordinates;
        all_unit_cube_coordinates.resize(_num_pieces);

        for (size_t i = 0; i < _num_pieces; i++) {

            auto piece = _puzzle[i];
            auto positions = piece.get_unit_cube_positions();
            utils::int3 translation = _positions[i];
                        
            for (size_t j = 0; j < positions.size(); j++) {
                utils::int3 pos = positions[j] + translation;
                
                all_unit_cube_coordinates[i].push_back(pos);
            }
        }

        return all_unit_cube_coordinates;
    }

    [[nodiscard]] const std::vector<utils::int3>& get_initial_positions() const noexcept
    {
        return _initial_positions;
    }

    [[nodiscard]] size_t get_dim() const noexcept
    {
        return _dim;
    }

    [[nodiscard]] size_t get_volume() const noexcept
    {
        return _volume;
    }

    [[nodiscard]] const glm::vec3& get_color(size_t index) const noexcept
    {
        return _colors[index];
    }

    [[nodiscard]] size_t get_num_pieces() const noexcept
    {
        return _num_pieces;
    }

    [[nodiscard]] bool is_solved() const noexcept
    {
        return _solved;
    }

    [[nodiscard]] double get_solve_time() const noexcept
    {
        return _solution_time;
    }

    [[nodiscard]] int get_nodes_visited() const noexcept
    {
        return _nodes_visited;
    }

    [[nodiscard]] int get_current_solution_state() const noexcept
    {
        return _displayed_solution_step;
    }

    [[nodiscard]] int get_solution_size() const noexcept
    {
        return static_cast<int>(_solution.size());
    }

    void set_solution_state(bool next)
    {
        if (next) {
            _displayed_solution_step = std::min({_displayed_solution_step + 1, static_cast<int>(_solution.size()) - 1});
        } else {
            _displayed_solution_step = std::max({_displayed_solution_step - 1, 0});
        }

        _build_field_from_node(_solution[_displayed_solution_step]);
        _positions = _solution[_displayed_solution_step].get_positions();
    }

    bool solve() noexcept
    {
        auto start = std::chrono::high_resolution_clock::now();

        std::unordered_set<std::vector<utils::int3>> visited;
        std::unordered_map<Node, Node> parents;
        std::priority_queue<Node> queue;

        visited.insert(_start.get_key());
        queue.push(_start);

        while (!queue.empty()) {
            Node current = queue.top();
            queue.pop();

            _build_field_from_node(current);

            if (_is_end_node(current)) {
                while (current != _start) {
                    _solution.push_back(current);
                    current = parents[current];
                }

                _solution.push_back(_start);
                std::ranges::reverse(_solution);

                _solved = true;
                _nodes_visited = static_cast<int>(visited.size());

                auto end = std::chrono::high_resolution_clock::now();
                _solution_time = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(end - start).count();

                return true;
            }

            for (const auto& neighbor : _get_neighbor_nodes(current)) {
                if (!visited.contains(neighbor.get_key())) {
                    visited.insert(neighbor.get_key());
                    parents[neighbor] = current;
                    queue.push(neighbor);
                }
            }
        }

        return false;
    }

private:
    void _build_field_from_node(const Node& node) noexcept
    {
        _field.reset();
        const auto& positions = node.get_positions();
        const auto& free_pieces = node.get_free_pieces();
        
        for (size_t i = 0; i < _num_pieces; i++) {
            if (free_pieces[i])
                continue;
            
            int index = utils::transform_index_3d_to_1d(positions[i], _dim);

            _field |= _puzzle[i].get_bitset(index);
        }
    }

    [[nodiscard]] std::vector<Node> _get_neighbor_nodes(const Node& node) const noexcept
    {
        std::vector<Node> neighbors;
        
        auto free_pieces = node.get_free_pieces();
        const auto& piece_positions = node.get_positions();
        
        size_t max_component_size = (_num_pieces - static_cast<size_t>(std::ranges::count(free_pieces, true))) / 2;

        _add_neighbor_nodes(neighbors, piece_positions, {-1, 0 , 0}, max_component_size, node);
        _add_neighbor_nodes(neighbors, piece_positions, {1, 0 , 0}, max_component_size, node);
        _add_neighbor_nodes(neighbors, piece_positions, {0, -1 , 0}, max_component_size, node);
        _add_neighbor_nodes(neighbors, piece_positions, {0, 1 , 0}, max_component_size, node);
        _add_neighbor_nodes(neighbors, piece_positions, {0, 0 , -1}, max_component_size, node);
        _add_neighbor_nodes(neighbors, piece_positions, {0, 0 , 1}, max_component_size, node);
        
        return neighbors;
    }

    void _add_neighbor_nodes(std::vector<Node>& neighbors, const std::vector<utils::int3>& piece_positions, utils::int3 direction, const size_t max_component_size, const Node& node) const
    {
        std::vector<bool> free_pieces = node.get_free_pieces();
        std::vector<utils::int3> new_positions = piece_positions;
        std::unordered_map<int, std::vector<int>> graph;

        int dim = -1;
        int sign = 0;
        
        for (int i = 0; i < 3; i++) {
            if (direction[i] != 0) {
                dim = i;
                if (direction[dim] > 0) sign = 1;
                else if (direction[dim] < 0) sign = -1;
                break;
            }
        }

        if (dim == -1 || sign == 0)
            throw std::runtime_error("Incompatible direction vector.");
        
        new_positions = piece_positions;
        
        for (size_t i = 0; i < _num_pieces; i++) {
            if (!free_pieces[i]) {
                graph[i] = _get_collisions(i, direction, node);
            }
        }

        std::vector<std::vector<int>> strongly_connected_components = _find_strongly_connected_components(graph);

        for (auto& component : strongly_connected_components) {
            if (component.size() <= max_component_size) {

                int max = 0;
                
                for (int unit = 0; unit < _dim; unit++) {
                    if (!_collides(component, piece_positions, direction * unit))
                        max = -unit;
                    else
                        break;
                }

                if (max < 0) {
                    bool is_piece_in_component_free = false;

                    for (int piece : component) {
                        if (new_positions[piece][dim] + max == (sign == -1 ? 0 : _dim - 1)) {
                            free_pieces[piece] = true;
                            is_piece_in_component_free = true;
                        }
                    }

                    for (int piece : component) {
                        if (is_piece_in_component_free)
                            new_positions[piece][dim] += max;
                        else
                            new_positions[piece][dim] += sign;
                    }

                    neighbors.push_back(Node(new_positions, _dim));
                }
            }
        }
    }

    [[nodiscard]] bool _collides(size_t piece, utils::int3 direction) const noexcept
    {
        utils::int3 new_position_3d = _positions[piece] + direction;

        if (new_position_3d.x < 0 || new_position_3d.x > _dim) return true;
        if (new_position_3d.y < 0 || new_position_3d.y > _dim) return true;
        if (new_position_3d.z < 0 || new_position_3d.z > _dim) return true;

        int new_position_1d = utils::transform_index_3d_to_1d(new_position_3d, _dim);

        std::bitset<N*N*N> temp(_field);
        temp ^= _puzzle[piece].get_bitset(_positions[piece]);

        if ((temp & _puzzle[piece].get_bitset(new_position_1d)).any()) {
            return true;
        }

        return false;
        
    }
    
    [[nodiscard]] bool _collides(std::vector<int> pieces, const std::vector<utils::int3>& piece_positions, utils::int3 direction) const noexcept
    {
        std::vector<int> new_positions;

        for (int piece : pieces) {
            utils::int3 position = piece_positions[piece];
            position += direction;

            if (position.x >= _dim || position.y >= _dim || position.z >= _dim)
                return true;
            
            if (position.x < 0 || position.y < 0 || position.z < 0)
                return true;

            int new_index = utils::transform_index_3d_to_1d(position, _dim);
            new_positions.push_back(new_index);
        }

        std::bitset<N*N*N> temp(_field);

        for (int piece : pieces) {
            int index = utils::transform_index_3d_to_1d(piece_positions[piece], _dim);
            temp ^= _puzzle[piece].get_bitset(index);
        }

        for (size_t piece = 0; piece < pieces.size(); piece++) {
            if ((temp & _puzzle[pieces[piece]].get_bitset(new_positions[piece])).any())
                return true;

            temp |= _puzzle[pieces[piece]].get_bitset(new_positions[piece]);
        }

        return false;
    }
    
    [[nodiscard]] std::vector<int> _get_collisions(size_t piece, utils::int3 direction, const Node& node) const noexcept
    {
        std::vector<int> collisions;

        auto free_pieces = node.get_free_pieces();
        auto positions = node.get_positions();
        auto new_position = positions[piece] + direction;

        if (new_position.x >= _dim || new_position.y >= _dim || new_position.z >= _dim) {
            return {};
        }

        if (new_position.x < 0 || new_position.y < 0 || new_position.z < 0) {
            return {};
        }

        int new_position_1d = utils::transform_index_3d_to_1d(new_position, _dim);
        
        for (size_t i = 0; i < _num_pieces; i++) {
            if (piece != i && !free_pieces[i]) {
                int old_position_1d = utils::transform_index_3d_to_1d(positions[i], _dim);
                
                if ((_puzzle[piece].get_bitset(new_position_1d) & _puzzle[i].get_bitset(old_position_1d)).any()) {
                    collisions.push_back(i);
                }
            }
        }

        return collisions;
    }

    [[nodiscard]] std::vector<std::vector<int>> _find_strongly_connected_components(const std::unordered_map<int, std::vector<int>>& graph) const noexcept
    {
        std::vector<bool> visited(_num_pieces, false);
        std::stack<int> stack;

        // Perform depth-first search and fill the stack with nodes in the order of their finish times
        for (const auto& key : graph | std::views::keys) {
            if (!visited[key]) {
                _depth_first_search_util(graph, key, visited, stack);
            }
        }

        // Reverse the edges of the graph
        std::unordered_map<int, std::vector<int>> reversed_graph;

        for (const auto& entry : graph) {
            int node = entry.first;
            reversed_graph[node];

            for (int neighbor : entry.second) {
                reversed_graph[neighbor].push_back(node);
            }
        }

        // Reset visited array
        visited.assign(_num_pieces, false);

        // Perform depth-first search on the reversed graph using nodes from the stack
        std::vector<std::vector<int>> strongly_connected_components;

        while (!stack.empty()) {
            int node = stack.top();
            stack.pop();

            if (!visited[node]) {
                std::vector<int> component;
                _depth_first_search(reversed_graph, node, visited, component);
                strongly_connected_components.push_back(component);
            }
        }

        return strongly_connected_components;
    }

    void _depth_first_search(const std::unordered_map<int, std::vector<int>>& graph, int node, std::vector<bool>& visited, std::vector<int>& component) const noexcept
    {
        visited[node] = true;
        component.push_back(node);

        for (int neighbor : graph.at(node)) {
            if (!visited[neighbor]) {
                _depth_first_search(graph, neighbor, visited, component);
            }
        }
    }

    void _depth_first_search_util(const std::unordered_map<int, std::vector<int>>& graph, int node, std::vector<bool>& visited, std::stack<int>& stack) const noexcept
    {
        visited[node] = true;

        for (int neighbor : graph.at(node)) {
            if (!visited[neighbor]) {
                _depth_first_search_util(graph, neighbor, visited, stack);
            }
        }

        stack.push(node);
    }
    
    [[nodiscard]] bool _is_end_node(const Node& node) const noexcept
    {
        std::vector<bool> free_pieces = node.get_free_pieces();
        uint32_t count = static_cast<uint32_t>(std::ranges::count(free_pieces, true));

        return count >= _num_pieces - 2;
    }

private:
    size_t _dim = N;
    size_t _volume = N*N*N;
    size_t _num_pieces = 0;

    Node _start;
    std::bitset<N*N*N> _field;
    std::vector<utils::int3> _initial_positions;
    std::vector<utils::int3> _positions;
    
    // Needs to be mutable because Piece<N>::get_bitset modifies this field
    mutable std::vector<Piece<N>> _puzzle;

    std::vector<glm::vec3> _colors = {
        {1.0f, 0.0f, 1.0f},
        {1.0f, 1.0f, 0.0f},
        {0.0f, 1.0f, 1.0f},
        {1.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 1.0f},
        {1.0f, 0.0f, 1.0f},
        {1.0f, 1.0f, 0.0f},
        {0.0f, 1.0f, 1.0f},
        {1.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 1.0f},
        {1.0f, 0.0f, 1.0f},
        {1.0f, 1.0f, 0.0f},
        {0.0f, 1.0f, 1.0f},
        {1.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 1.0f}
    };

    bool _solved = false;
    double _solution_time = 0.0;
    int _nodes_visited = 0;
    int _displayed_solution_step = 0;
    std::vector<Node> _solution;
};
