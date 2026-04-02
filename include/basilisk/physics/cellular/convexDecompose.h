#ifndef BSK_PHYSICS_CELLULAR_CONVEX_DECOMPOSE_H
#define BSK_PHYSICS_CELLULAR_CONVEX_DECOMPOSE_H

#include <basilisk/util/includes.h>

#include <vector>

namespace bsk::internal {

// ----------------------------------------------
// Convex
// ----------------------------------------------

class Convex {
public:
    std::vector<glm::vec2> vertices;

    Convex(const glm::vec2& a, const glm::vec2& b, const glm::vec2& c);
};

}

#endif
