#pragma once

#include <vector>

#include "utils.h"

class Node final
{
public:
    Node() = default;
    Node(std::vector<utils::int3> positions, int dim) noexcept;

    [[nodiscard]] const std::vector<utils::int3>& get_positions() const noexcept;
    [[nodiscard]] const std::vector<utils::int3>& get_key() const noexcept;
    [[nodiscard]] const std::vector<bool>& get_free_pieces() const noexcept;

    [[nodiscard]] bool operator==(const Node&) const;
    [[nodiscard]] bool operator!=(const Node&) const;
    [[nodiscard]] bool operator<(const Node&) const;

private:
    void _calculate_priority() noexcept;
    void _calculate_free_pieces() noexcept;
    void _calculate_min() noexcept;
    void _calculate_key() noexcept;
    
private:
    int _dim;
    int _priority;

    std::vector<utils::int3> _positions;
    std::vector<utils::int3> _key;

    int _distance_to_edge = 3;
    std::vector<bool> _free_pieces;

    utils::int3 _min;
};
