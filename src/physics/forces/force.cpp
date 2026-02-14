#include <basilisk/physics/forces/force.h>
#include <basilisk/physics/rigid.h>
#include <basilisk/physics/solver.h>
#include <basilisk/physics/tables/forceTable.h>

namespace bsk::internal {

Force::Force(Solver* solver, Rigid* bodyA, Rigid* bodyB)
    : solver(solver), forceTable(solver->getForceTable()), bodyA(bodyA), bodyB(bodyB), next(nullptr), nextA(nullptr), nextB(nullptr), prev(nullptr), prevA(nullptr), prevB(nullptr)
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
    if (bodyA) {
        bodyA->remove(this);
    }

    if (bodyB) {
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
bsk::vec3& Force::getJ(int index) const { return forceTable->getJ(this->index, index); }
bsk::mat3x3& Force::getH(int index) const { return forceTable->getH(this->index, index); }
float Force::getC(int index) const { return forceTable->getC(this->index, index); }
float Force::getFmin(int index) const { return forceTable->getFmin(this->index, index); }
float Force::getFmax(int index) const { return forceTable->getFmax(this->index, index); }
float Force::getStiffness(int index) const { return forceTable->getStiffness(this->index, index); }
float Force::getFracture(int index) const { return forceTable->getFracture(this->index, index); }
float Force::getPenalty(int index) const { return forceTable->getPenalty(this->index, index); }
float Force::getLambda(int index) const { return forceTable->getLambda(this->index, index); }

bsk::vec3& Force::getPosA() const { return forceTable->getPosA(index); }
bsk::vec3& Force::getPosB() const { return forceTable->getPosB(index); }
bsk::vec3& Force::getInitialA() const { return forceTable->getInitialA(index); }
bsk::vec3& Force::getInitialB() const { return forceTable->getInitialB(index); }
ForceType Force::getForceType() const { return forceTable->getForceType(index); }

// setters
void Force::setJ(int index, const glm::vec3& value) { forceTable->setJ(this->index, index, value); }
void Force::setH(int index, const glm::mat3& value) { forceTable->setH(this->index, index, value); }
void Force::setC(int index, float value) { forceTable->setC(this->index, index, value); }
void Force::setFmin(int index, float value) { forceTable->setFmin(this->index, index, value); }
void Force::setFmax(int index, float value) { forceTable->setFmax(this->index, index, value); }
void Force::setStiffness(int index, float value) { forceTable->setStiffness(this->index, index, value); }
void Force::setFracture(int index, float value) { forceTable->setFracture(this->index, index, value); }
void Force::setPenalty(int index, float value) { forceTable->setPenalty(this->index, index, value); }
void Force::setLambda(int index, float value) { forceTable->setLambda(this->index, index, value); }

void Force::setPosA(const glm::vec3& value) { forceTable->setPosA(index, value); }
void Force::setPosB(const glm::vec3& value) { forceTable->setPosB(index, value); }
void Force::setInitialA(const glm::vec3& value) { forceTable->setInitialA(index, value); }
void Force::setInitialB(const glm::vec3& value) { forceTable->setInitialB(index, value); }
void Force::setForceType(ForceType value) { forceTable->setForceType(index, value); }
}