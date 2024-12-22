#pragma once

#include <stdexcept>
#include <glm/glm.hpp>

namespace utils
{
    struct int3
    {
        int x, y, z;

        bool operator==(const int3& other) const
        {
            if (x != other.x | y != other.y | z != other.z)
                return false;

            return true;
        }

        int3 operator+(const int3& other) const
        {
            return { x + other.x, y + other.y, z + other.z };
        }
        
        int3& operator+=(const int3& other)
        {
            this->x += other.x;
            this->y += other.y;
            this->z += other.z;

            return *this;
        }

        int3 operator*(int other) const
        {
            return { other * x, other * y, other * z };
        }

        int& operator[](size_t index)
        {
            if (index == 0)
                return x;
            if (index == 1)
                return y;
            if (index == 2)
                return z;

            throw std::runtime_error("utils::int3: Index out of range.");
        }
        
        operator glm::vec3() const
        {
            return {static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)};
        }
    };
    
    inline int3 transform_index_1d_to_3d(int i, int dim)
    {
        int z = i / (dim * dim);
        int y = (i - z * dim * dim) / dim;
        int x = (i - z * dim * dim - y * dim);

        return { x, y, z };
    }

    inline int transform_index_3d_to_1d(int x, int y, int z, int dim)
    {
        return x + y * dim + z * dim * dim;
    }

    inline int transform_index_3d_to_1d(const int3& vec, int dim)
    {
        return vec.x + vec.y * dim + vec.z * dim * dim;
    }
}
