#ifndef BSK_GJK_H
#define BSK_GJK_H

#include <basilisk/util/includes.h>
#include <basilisk/util/constants.h>

namespace bsk::internal {

// ------------------------------------------------------------
// Structs N Stuff
// ------------------------------------------------------------

// NOTE assumes gc is near pos
struct ConvexShape {
    const std::vector<glm::vec2>& vertices;

    glm::vec2 pos;
    float rot;
    glm::vec2 scale;

    ConvexShape(const std::vector<glm::vec2>& vertices, glm::vec2 pos, float rot, glm::vec2 scale)
        : vertices(vertices), pos(pos), rot(rot), scale(scale) {}
};

using Simplex = std::array<glm::vec2, 3>;

// // add 3 since the simplex starts with 3 vertices
// using SpSet = std::array<ushort, EPA_ITERATIONS + 3>;
// using SpArray = std::array<glm::vec2, EPA_ITERATIONS + 3>;
// using Polytope = std::array<PolytopeFace, EPA_ITERATIONS + 3>;

struct CollisionPair {
    // gjk
    Simplex simplex;
    glm::vec2 searchDir;
    uint32_t freeIndex = 0;

    // epa
    // SpArray sps;
    // SpSet spSet;
    // Polytope polytope;

    CollisionPair() = default;
};

// ------------------------------------------------------------
// Helper Methods
// ------------------------------------------------------------

void addSupport(const ConvexShape& shapeA, const ConvexShape& shapeB, CollisionPair& pair, uint32_t insertIndex);
glm::vec2 getFar(const ConvexShape& shape, const glm::vec2& dir);

// ------------------------------------------------------------
// GJK
// ------------------------------------------------------------

bool gjk(const ConvexShape& shapeA, const ConvexShape& shapeB, CollisionPair& pair);

uint32_t handleSimplex(const ConvexShape& shapeA, const ConvexShape& shapeB, CollisionPair& pair, uint32_t freeIndex);
uint32_t handle0(const ConvexShape& shapeA, const ConvexShape& shapeB, CollisionPair& pair);
uint32_t handle1(const ConvexShape& shapeA, const ConvexShape& shapeB, CollisionPair& pair);
uint32_t handle2(const ConvexShape& shapeA, const ConvexShape& shapeB, CollisionPair& pair);
uint32_t handle3(const ConvexShape& shapeA, const ConvexShape& shapeB, CollisionPair& pair);

// struct PolytopeFace {
//     glm::vec2 normal;
//     float distance;
//     ushort va;
//     ushort vb;

//     PolytopeFace() = default;
//     PolytopeFace(ushort va, ushort vb, glm::vec2 normal, float distance)
//         : normal(normal), distance(distance), va(va), vb(vb) {}
// };

// bool gjk(Rigid* bodyA, Rigid* bodyB, CollisionPair& pair);

}

#endif