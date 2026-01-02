#ifndef BSK_GJK_H
#define BSK_GJK_H

#include <basilisk/util/includes.h>

namespace bsk::internal {

class Rigid;
class CollisionPair;

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