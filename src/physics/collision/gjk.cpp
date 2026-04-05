#include <basilisk/physics/collision/gjk.h>
// #include <basilisk/physics/maths.h>
#include <basilisk/util/maths.h>

namespace bsk::internal {

void addSupport(const ConvexShape& shapeA, const ConvexShape& shapeB, CollisionPair& pair, uint32_t insertIndex) {
    glm::vec2 farA = getFar(shapeA, pair.searchDir);
    glm::vec2 farB = getFar(shapeB, -pair.searchDir);
    pair.sps[insertIndex] = farA - farB;
}

glm::vec2 getFar(const ConvexShape& shape, const glm::vec2& dir) {
    glm::vec2 maxVertex = shape.vertices[0];
    float maxDot = glm::dot(maxVertex, dir);
    for (uint32_t i = 1; i < shape.vertices.size(); ++i) {
        float d = glm::dot(shape.vertices[i], dir);
        if (d > maxDot) {
            maxDot = d;
            maxVertex = shape.vertices[i];
        }
    }
    return maxVertex;
}

// ------------------------------------------------------------
// GJK
// ------------------------------------------------------------

bool gjk(const ConvexShape& shapeA, const ConvexShape& shapeB, CollisionPair& pair) {
    for (uint32_t i = 0; i < GJK_ITERATIONS; ++i) {
        pair.freeIndex = handleSimplex(shapeA, shapeB, pair, pair.freeIndex);
        if (pair.freeIndex == -1) {
            return true;
        }
        addSupport(shapeA, shapeB, pair, pair.freeIndex);
        if (glm::dot(pair.sps[pair.freeIndex], pair.searchDir) < EPSILON) {
            return false;
        }
        pair.freeIndex++;
    }
    return false;
}

uint32_t handleSimplex(const ConvexShape& shapeA, const ConvexShape& shapeB, CollisionPair& pair, uint32_t freeIndex) {
    switch (freeIndex) {
        case 0: return handle0(shapeA, shapeB, pair);
        case 1: return handle1(shapeA, shapeB, pair);
        case 2: return handle2(shapeA, shapeB, pair);
        case 3: return handle3(shapeA, shapeB, pair);
        default: throw std::runtime_error("simplex has incorrect freeIndex");
    }
}

uint32_t handle0(const ConvexShape& shapeA, const ConvexShape& shapeB, CollisionPair& pair) {
    pair.searchDir = shapeB.pos - shapeA.pos;
    return 0;
}

uint32_t handle1(const ConvexShape& shapeA, const ConvexShape& shapeB, CollisionPair& pair) {
    pair.searchDir = -pair.sps[0];
    return 1;
}

uint32_t handle2(const ConvexShape& shapeA, const ConvexShape& shapeB, CollisionPair& pair) {
    glm::vec2 CB = pair.sps[1] - pair.sps[0];
    glm::vec2 CO =               - pair.sps[0];
    tripleProduct(CB, CO, CB, pair.searchDir);

    if (glm::length2(pair.searchDir) < EPSILON) {
        // fallback perpendicular
        perpTowards(CB, CO, pair.searchDir);
    }

    return 2;
}

uint32_t handle3(const ConvexShape& shapeA, const ConvexShape& shapeB, CollisionPair& pair) {
    glm::vec2 AB = pair.sps[1] - pair.sps[2];
    glm::vec2 AC = pair.sps[0] - pair.sps[2];
    glm::vec2 AO =               - pair.sps[2];
    glm::vec2 CO =               - pair.sps[0];

    // remove 0
    glm::vec2 perp;
    perpTowards(AB, CO, perp);
    if (glm::dot(perp, AO) > -EPSILON) {
        pair.sps[0] = pair.sps[2];
        pair.searchDir = perp;
        return 2;
    }

    // remove 1
    glm::vec2 BO = -pair.sps[1];
    perpTowards(AC, BO, perp);
    if (glm::dot(perp, AO) > -EPSILON) {
        pair.sps[1] = pair.sps[2];
        pair.searchDir = perp;
        return 2;
    }

    // we have found a collision
    return -1;
}

}