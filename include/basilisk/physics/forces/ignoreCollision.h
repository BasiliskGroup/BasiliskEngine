#ifndef BSK_PHYSICS_FORCES_IGNORE_COLLISION_H
#define BSK_PHYSICS_FORCES_IGNORE_COLLISION_H

#include <basilisk/physics/forces/force.h>

namespace bsk::internal {
    
// Force which has no physical effect, but is used to ignore collisions between two bodies
class IgnoreCollision : public Force {
public:
    IgnoreCollision(Solver* solver, Rigid* bodyA, Rigid* bodyB)
        : Force(solver, bodyA, bodyB) {}

    static int rows(ForceTable* forceTable, std::size_t index) { return 0; }
    int rows() override { return 0; }
    bool initialize() override { return true; }
    static void computeConstraint(ForceTable* forceTable, std::size_t index, float alpha) {}
    static void computeDerivatives(ForceTable* forceTable, std::size_t index, ForceBodyOffset body) {}
};

}

#endif