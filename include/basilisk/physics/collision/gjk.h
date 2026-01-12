#ifndef BSK_GJK_H
#define BSK_GJK_H

#include <basilisk/util/includes.h>
#include <basilisk/util/constants.h>

namespace bsk::internal {

class Rigid;

struct PolytopeFace {
    glm::vec2 normal;
    float distance;
    ushort va;
    ushort vb;

    PolytopeFace() = default;
    PolytopeFace(ushort va, ushort vb, glm::vec2 normal, float distance)
        : normal(normal), distance(distance), va(va), vb(vb) {}
};

using Simplex = std::array<glm::vec2, 3>;

// add 3 since the simplex starts with 3 vertices
using SpSet = std::array<ushort, EPA_ITERATIONS + 3>;
using SpArray = std::array<glm::vec2, EPA_ITERATIONS + 3>;
using Polytope = std::array<PolytopeFace, EPA_ITERATIONS + 3>;

struct CollisionPair {
    // gjk
    Simplex simplex;
    glm::vec2 searchDir;

    // epa // TODO reuse this memory for multiple collision pairs
    SpArray sps;
    SpSet spSet;
    Polytope polytope;

    CollisionPair() = default;
};

bool gjk(Rigid* bodyA, Rigid* bodyB, CollisionPair& pair);

std::size_t handleSimplex(Rigid* bodyA, Rigid* bodyB, CollisionPair& pair, std::size_t freeIndex);
std::size_t handle0(Rigid* bodyA, Rigid* bodyB, CollisionPair& pair);
std::size_t handle1(Rigid* bodyA, Rigid* bodyB, CollisionPair& pair);
std::size_t handle2(Rigid* bodyA, Rigid* bodyB, CollisionPair& pair);
std::size_t handle3(Rigid* bodyA, Rigid* bodyB, CollisionPair& pair);
void addSupport(Rigid* bodyA, Rigid* bodyB, CollisionPair& pair, std::size_t insertIndex);
void getFar(const Rigid* body, const glm::vec2& dir, glm::vec2& simplexLocal);

}

#endif