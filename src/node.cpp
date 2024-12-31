#include "node.h"
#include <iostream>

Node::Node(std::vector<utils::int3> positions, int dim) noexcept
    : _dim(dim), _positions(std::move(positions)) 
{
    _calculate_priority();
    _calculate_free_pieces();
    _calculate_min();
    _calculate_key();
}

const std::vector<utils::int3>& Node::get_positions() const noexcept
{
    return _positions;
}

const std::vector<utils::int3>& Node::get_key() const noexcept
{
    return _key;
}

const std::vector<bool>& Node::get_free_pieces() const noexcept
{
    return _free_pieces;
}

bool Node::operator==(const Node& other) const
{
    for (size_t i = 0; i < _positions.size(); i++) {
        if (_positions[i] != other._positions[i])
            return false;
    }

    return true;
}

bool Node::operator!=(const Node& other) const
{
    for (size_t i = 0; i < _positions.size(); i++) {
        if (_positions[i] != other._positions[i])
            return true;
    }

    return false;
}

bool Node::operator<(const Node& other) const
{
    // Reversed as we want the lowest value at the top of the priority queue
    return _priority > other._priority;
}

void Node::print_positions() const noexcept {
    for (size_t i = 0; i < _positions.size(); i++) {
        std::cout << "Piece " << i << ": " << _positions[i].x << ", " << _positions[i].y << ", " << _positions[i].z << " " << std::endl;
    }
    std::cout << std::endl;
}

void Node::_calculate_priority() noexcept
{
    _priority = 0;
    
    for (auto& _position : _positions) {
        _priority += std::min({_position.x, _position.y, _position.z, _dim - _position.x, _dim - _position.y, _dim - _position.z});
    }
}

void Node::_calculate_free_pieces() noexcept
{
    for (auto& position : _positions) {
        if (position.x < _distance_to_edge || position.x > _dim - _distance_to_edge) {
            _free_pieces.push_back(true);
            continue;
        }
        
        if (position.y < _distance_to_edge || position.y > _dim - _distance_to_edge) {
            _free_pieces.push_back(true);
            continue;
        }

        if (position.z < _distance_to_edge || position.z > _dim - _distance_to_edge) {
            _free_pieces.push_back(true);
            continue;
        }

        _free_pieces.push_back(false);
    }
}

void Node::_calculate_min() noexcept
{
    _min.x = std::numeric_limits<int>::max();
    _min.y = std::numeric_limits<int>::max();
    _min.z = std::numeric_limits<int>::max();    
    
    for (size_t i = 0; i < _positions.size(); i++) {
        if (!_free_pieces[i]) {
            _min.x = std::min(_positions[i].x, _min.x);
            _min.y = std::min(_positions[i].y, _min.y);
            _min.z = std::min(_positions[i].z, _min.z);
        }
    }
}

void Node::_calculate_key() noexcept
{
    for (size_t i = 0; i < _positions.size(); i++) {
        if (!_free_pieces[i]) {
            _key.push_back({_positions[i].x - _min.x, _positions[i].y - _min.y, _positions[i].z - _min.z});
        } else {
            _key.push_back({0, 0, 0});
        }
    }
}
