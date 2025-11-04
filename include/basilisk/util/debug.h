#ifndef BSK_DEBUG_H
#define BSK_DEBUG_H

#include <glm/glm.hpp>
#include <cmath> // for std::isnan

namespace bsk::internal {

// Generic function to detect NaNs in any glm type
template <typename T>
bool hasNaN(const T& value) {
    // For scalars
    if constexpr (std::is_arithmetic_v<T>) {
        return std::isnan(value);
    }
    // For glm vector and matrix types
    else {
        for (int i = 0; i < T::length(); ++i) {
            if (hasNaN(value[i]))
                return true;
        }
        return false;
    }
}

}

#endif