/*
* Copyright (c) 2025 Chris Giles
*
* Permission to use, copy, modify, distribute and sell this software
* and its documentation for any purpose is hereby granted without fee,
* provided that the above copyright notice appear in all copies.
* Chris Giles makes no representations about the suitability
* of this software for any purpose.
* It is provided "as is" without express or implied warranty.
*/

#include <basilisk/physics/forces/force.h>
#include <basilisk/physics/rigid.h>
#include <basilisk/physics/solver.h>
#include <basilisk/physics/tables/forceTable.h>

namespace bsk::internal {

Force::Force(Solver* solver, Rigid* bodyA, Rigid* bodyB)
    : solver(solver), bodyA(bodyA), bodyB(bodyB), next(nullptr), nextA(nullptr), nextB(nullptr), prev(nullptr), prevA(nullptr), prevB(nullptr)
{
    // Add to solver linked list
    solver->insert(this);
    solver->getForceTable()->insert(this);
    solver->getForceTable()->setForceType(this->index, ForceType::NULL_FORCE);

    // Add to body linked lists
    if (bodyA) {
        bodyA->insert(this);
    }
    if (bodyB) {
        bodyB->insert(this);
    }
}

Force::~Force() {
    // Remove from solver linked list
    solver->remove(this);
    solver->getForceTable()->markAsDeleted(this->index);

    // Remove from body linked lists
    if (bodyA)
    {
        bodyA->remove(this);
    }

    if (bodyB)
    {
        bodyB->remove(this);
    }

    // Clean up pointers
    bodyA = nullptr;
    bodyB = nullptr;
    solver = nullptr;
}

void Force::disable() {
    // Disable this force by clearing the relavent fields
    static const std::array<float, MAX_ROWS> zeros = { 0 }; // zero initialized array

    for (int i = 0; i < MAX_ROWS; i++) {
        setStiffness(i, 0.0f);
        setPenalty(i, 0.0f);
        setLambda(i, 0.0f);
    }
}

// getters
glm::vec3& Force::getJ(int index, Rigid* body) const { 
    return (body == bodyA) ? 
        solver->getForceTable()->getJA(this->index, index) : 
        solver->getForceTable()->getJB(this->index, index); 
}

glm::mat3x3& Force::getH(int index, Rigid* body) const { 
    return (body == bodyA) ? 
        solver->getForceTable()->getHA(this->index, index) : 
        solver->getForceTable()->getHB(this->index, index); 
}

float Force::getC(int index) const { return solver->getForceTable()->getC(this->index, index); }
float Force::getFmin(int index) const { return solver->getForceTable()->getFmin(this->index, index); }
float Force::getFmax(int index) const { return solver->getForceTable()->getFmax(this->index, index); }
float Force::getStiffness(int index) const { return solver->getForceTable()->getStiffness(this->index, index); }
float Force::getFracture(int index) const { return solver->getForceTable()->getFracture(this->index, index); }
float Force::getPenalty(int index) const { return solver->getForceTable()->getPenalty(this->index, index); }
float Force::getLambda(int index) const { return solver->getForceTable()->getLambda(this->index, index); }

// setters
void Force::setJ(int index, Rigid* body, const glm::vec3& value) { 
    if (body == bodyA) { solver->getForceTable()->setJA(this->index, index, value); } 
    else { solver->getForceTable()->setJB(this->index, index, value); } 
}

void Force::setH(int index, Rigid* body, const glm::mat3& value) { 
    if (body == bodyA) { solver->getForceTable()->setHA(this->index, index, value); } 
    else { solver->getForceTable()->setHB(this->index, index, value); } 
}

// index specific
void Force::setC(int index, float value) { solver->getForceTable()->setC(this->index, index, value); }
void Force::setFmin(int index, float value) { solver->getForceTable()->setFmin(this->index, index, value); }
void Force::setFmax(int index, float value) { solver->getForceTable()->setFmax(this->index, index, value); }
void Force::setStiffness(int index, float value) { solver->getForceTable()->setStiffness(this->index, index, value); }
void Force::setFracture(int index, float value) { solver->getForceTable()->setFracture(this->index, index, value); }
void Force::setPenalty(int index, float value) { solver->getForceTable()->setPenalty(this->index, index, value); }
void Force::setLambda(int index, float value) { solver->getForceTable()->setLambda(this->index, index, value); }
}