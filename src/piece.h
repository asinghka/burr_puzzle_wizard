#pragma once

#include <bitset>
#include <unordered_map>

#include "utils.h"

template<size_t N>
class Piece final
{
public:
    Piece(std::bitset<N * N * N> bits) noexcept
    {
        _piece = bits;
        _generate_positions();
    }

    [[nodiscard]] const std::bitset<N*N*N>& get_bitset(int i) noexcept
    {
        auto key = _pieces.find(i);

        if (key == _pieces.end()) {
            _pieces[i] = _piece << i;
        }

        return _pieces[i];
    }

    [[nodiscard]] const std::bitset<N*N*N>& get_bitset(utils::int3 pos) noexcept
    {
        int i = utils::transform_index_3d_to_1d(pos, _dim);
        auto key = _pieces.find(i);

        if (key == _pieces.end()) {
            _pieces[i] = _piece << i;
        }

        return _pieces[i];
    }
    
    [[nodiscard]] size_t get_num_unit_cubes() const noexcept
    {
        return _positions.size();
    }

    [[nodiscard]] const std::vector<utils::int3>& get_unit_cube_positions() const noexcept
    {
        return _positions;
    }
    
private:
    void _generate_positions() noexcept
    {
        for (int i = 0; i < _volume; i++) {
            if (_piece[i]) {
                utils::int3 pos = utils::transform_index_1d_to_3d(i, N);
                _positions.push_back(pos);
            }
        }
    }
    
private:
    size_t _dim = N;
    size_t _volume = N*N*N;

    std::bitset<N*N*N> _piece;
    std::unordered_map<uint32_t, std::bitset<N*N*N>> _pieces;

    std::vector<utils::int3> _positions;
};