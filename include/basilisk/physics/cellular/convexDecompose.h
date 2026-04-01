#ifndef BSK_PHYSICS_CELLULAR_CONVEX_DECOMPOSE_H
#define BSK_PHYSICS_CELLULAR_CONVEX_DECOMPOSE_H

#include <basilisk/util/includes.h>
#include <basilisk/physics/cellular/helper.h>

namespace bsk::internal {

// ----------------------------------------------
// Convex
// ----------------------------------------------

class Convex {
private:
    // Directed boundary edge u->v maps to the index of v in `vertices` (insert splits u->v by
    // inserting before v). Rebuilt from the full ring after every change so indices stay valid.
    std::unordered_map<Edge, int, EdgeHash> edgeToIndex;
    std::vector<glm::vec2> vertices;

    void rebuildEdges();

public:
    Convex(const glm::vec2& a, const glm::vec2& b, const glm::vec2& c) : vertices{a, b, c} { rebuildEdges(); }

    bool add(const glm::vec2& a, const glm::vec2& b, const glm::vec2& c);

    const std::vector<glm::vec2>& ring() const { return vertices; }
};

}

#endif