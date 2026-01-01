#ifndef BSK_GJK_H
#define BSK_GJK_H

#include <basilisk/util/includes.h>

namespace bsk::internal {

class Rigid;

using Simplex = std::array<glm::vec2, 3>;

bool gjk(Rigid* bodyA, Rigid* bodyB);

std::size_t handleSimplex(Simplex& simplex, std::size_t freeIndex);
std::size_t handle0(Rigid* bodyA, Rigid* bodyB, Simplex& simplex);
std::size_t handle1(Rigid* bodyA, Rigid* bodyB, Simplex& simplex);
std::size_t handle2(Rigid* bodyA, Rigid* bodyB, Simplex& simplex);
std::size_t handle3(Rigid* bodyA, Rigid* bodyB, Simplex& simplex);
void addSupport(Rigid* bodyA, Rigid* bodyB, Simplex& simplex, std::size_t insertIndex);
void getFar(const Rigid* body, const glm::vec2& dir, glm::vec2& simplexLocal);

}

#endif