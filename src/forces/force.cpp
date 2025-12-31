#include "basilisk/util/includes.h"
#include <basilisk/solver/physics.h>

namespace bsk::internal {

Force::Force(Solver* solver, Rigid* bodyA, Rigid* bodyB) : 
    solver(solver), 
    next(nullptr), 
    bodyA(bodyA), 
    bodyB(bodyB), 
    nextA(nullptr), 
    nextB(nullptr), 
    prev(nullptr), 
    prevA(nullptr), 
    prevB(nullptr) 
{
    solver->insert(this);
    bodyA->insert(this);
    if (bodyB) {
        bodyB->insert(this);
    }

    for (uint i = 0; i < MANIFOLD_ROWS; i++) {
        getJ()[i] = { 0, 0, 0 };
        getH()[i] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
        getC()[i] = 0.0f;
        getStiffness()[i] = std::numeric_limits<float>::infinity();
        getFmax()[i] = std::numeric_limits<float>::infinity();
        getFmin()[i] = -std::numeric_limits<float>::infinity();
        getFracture()[i] = std::numeric_limits<float>::infinity();

        getPenalty()[i] = 0.0f;
        getLambda()[i] = 0.0f;
    }

    type = NULL_FORCE;
}

Force::~Force()
{
    solver->remove(this);
    bodyA->remove(this);
    if (bodyB) {
        bodyB->remove(this);
    }

    // clean up pointers
    bodyA = nullptr;
    bodyB = nullptr;
    solver = nullptr;
}

void Force::markAsDeleted() {
    // solver->getForceTable()->markAsDeleted(index);
}

void Force::disable() {
    // TODO disable force
}

ForceType& Force::getType() { return type; } // TODO allow for multiple types

}