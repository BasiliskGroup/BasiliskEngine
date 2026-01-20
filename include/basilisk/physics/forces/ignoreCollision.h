#pragma once

#include <basilisk/physics/forces/force.h>

namespace bsk::internal {
    
// Force which has no physical effect, but is used to ignore collisions between two bodies
class IgnoreCollision : public Force {
public:
    IgnoreCollision(Solver* solver, Rigid* bodyA, Rigid* bodyB)
        : Force(solver, bodyA, bodyB) {}

    int rows() const override { return 0; }

    bool initialize() override { return true; }
    static void computeConstraint(ForceTable* forceTable, std::size_t index, float alpha) {}
    static void computeDerivatives(ForceTable* forceTable, std::size_t index, ForceBodyOffset body) {}
};

}

