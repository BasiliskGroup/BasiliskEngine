#include <basilisk/solver/physics.h>

namespace bsk::internal {

Force::Force(Solver* solver, Rigid* bodyA, Rigid* bodyB) 
: solver(solver), next(nullptr), bodyA(bodyA), bodyB(bodyB), nextA(nullptr), twin(nullptr), prev(nullptr), prevA(nullptr) {
    solver->insert(this);
    bodyA->insert(this);

    // TODO set default params
    // Probably flag Table to set columns
}

Force::~Force()
{
    markAsDeleted(); // clean up in table
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
    solver->getForceTable()->markAsDeleted(index);
}

void Force::disable() {
    // TODO disable force
}

ForceTable* Force::getTable() { return solver->getForceTable(); }

BskVec3ROWS& Force::J() { return getTable()->getJ()[index]; }
BskMat3x3ROWS& Force::H() { return getTable()->getH()[index]; }
BskFloatROWS& Force::C() { return getTable()->getC()[index]; }
BskFloatROWS& Force::motor() { return getTable()->getMotor()[index]; }
BskFloatROWS& Force::stiffness() { return getTable()->getStiffness()[index]; }
BskFloatROWS& Force::fracture() { return getTable()->getFracture()[index]; }
BskFloatROWS& Force::fmax() { return getTable()->getFmax()[index]; }
BskFloatROWS& Force::fmin() { return getTable()->getFmin()[index]; }
BskFloatROWS& Force::penalty() { return getTable()->getPenalty()[index]; }
BskFloatROWS& Force::lambda() { return getTable()->getLambda()[index]; }

ForceType& Force::getType() { return getTable()->getType()[index]; }

uint& Force::getSpecialIndex() { return getTable()->getSpecial()[index]; }
uint& Force::getBodyIndex() { return getTable()->getBodyIndex()[index]; }
bool Force::isA() { return getTable()->getIsA()[index]; }

}