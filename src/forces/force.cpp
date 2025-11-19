#include <basilisk/solver/physics.h>

namespace bsk::internal {

Force::Force(Solver* solver, Rigid* bodyA, Rigid* bodyB) : 
    solver(solver), 
    next(nullptr), 
    bodyA(bodyA), 
    bodyB(bodyB), 
    nextA(nullptr), 
    twin(nullptr), 
    prev(nullptr), 
    prevA(nullptr),
    type(NULL_FORCE)
{
    solver->insert(this);
    bodyA->insert(this);    
}

Force::~Force()
{
    solver->remove(this);
    bodyA->remove(this);

    // delete the twin if it has not already been deleted, force twins must exist in pairs
    if (twin != nullptr) {

        // need to mark twin->this as nullptr so twin doesn't call this delete
        twin->twin = nullptr;
        delete twin;
    }

    // clean up pointers
    // TODO check if these are alraedy cleared by remove functions
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