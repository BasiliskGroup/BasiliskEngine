#include <basilisk/solver/physics.h>

namespace bsk::internal {

Rigid::Rigid(Solver* solver, Node2D* node, glm::vec3 pos, glm::vec2 scale, float density, float friction, glm::vec3 vel, Collider* collider) : 
    solver(solver), 
    node(node), 
    forces(nullptr), 
    next(nullptr), 
    prev(nullptr), 
    density(density) 
{
    // Add to linked list
    solver->insert(this);

    // compute intermediate values
    float volume = 1; // replace with collider volume
    float mass = scale.x * scale.y * density * volume; 
    float moment = mass * glm::dot(scale, scale) / 12.0f; // TODO replace with collider moment
    float radius = glm::length(scale * 0.5f);

    index = solver->getBodyTable()->insert(this, pos, vel, scale, friction, mass, moment, collider->getIndex(), radius);
}   

Rigid::Rigid(Solver* solver, Node2D* node, glm::vec3 pos, glm::vec2 scale, float density, float friction, glm::vec3 vel, uint collider) : 
    solver(solver), 
    node(node), 
    forces(nullptr), 
    next(nullptr), 
    prev(nullptr), 
    density(density) 
{
    // Add to linked list
    solver->insert(this);

    // compute intermediate values
    float volume = 1; // replace with collider volume
    float mass = scale.x * scale.y * density * volume; 
    float moment = mass * glm::dot(scale, scale) / 12.0f; // TODO replace with collider moment
    float radius = glm::length(scale * 0.5f);

    index = solver->getBodyTable()->insert(this, pos, vel, scale, friction, mass, moment, collider, radius);
}   

Rigid::~Rigid() {
    // remove from Table
    solver->getBodyTable()->markAsDeleted(index);
    solver->remove(this);

    // delete all forces
    Force* curForce = forces;
    while (curForce) {
        Force* nextForce = curForce->getNextA();
        delete curForce;
        curForce = nextForce;
    }
}

BodyTable* Rigid::getBodyTable() {
    return solver->getBodyTable();
}



uint Rigid::getColliderIndex() {
    return getBodyTable()->getCollider()[index];
}

// ----------------------
// Linked list management
// ----------------------

void Rigid::insert(Force* force){
    if (force == nullptr) {
        return;
    }

    force->getNextA() = forces;
    force->getPrevA() = nullptr;
    force->getBodyA() = this; // NOTE this may not be needed

    if (forces) {
        forces->getPrevA() = force;
    }

    forces = force;
}

void Rigid::remove(Force* force){
    if (force == nullptr || force->getBodyA() != this) {
        return;
    }

    if (force->getPrevA()) {
        force->getPrevA()->getNextA() = force->getNextA();
    } else {
        forces = force->getNextA();
    }

    if (force->getNextA()) {
        force->getNextA()->getPrevA() = force->getPrevA();
    }

    force->getPrevA() = nullptr;
    force->getNextA() = nullptr;
}

// ----------------------
// Graph
// ----------------------

void Rigid::precomputeRelations() {
    relations.clear();

    uint i = 0;
    for (Force* f = forces; f != nullptr; f = f->getNextA()) {
        Rigid* other = f->getBodyB();
        if (other == nullptr) {
            continue;
        }

        relations.emplace_back(other->getIndex(), f, f->getType());
    }
}

// ----------------------
// Broad Collision
// ----------------------

ForceType Rigid::constrainedTo(uint other, Force*& force) const {
    // check if this body is constrained to the other body
    for (const auto& rel : relations) {
        if (rel.bodyB == other) {
            force = rel.force;
            return rel.type;
        }
    }
    return NULL_FORCE;
}

void Rigid::draw() {

}

void Rigid::clear() {
    if (node != nullptr) {
        delete node;
        node = nullptr;
    }
}

// --------------------
// setters
// --------------------
void Rigid::setPosition(const glm::vec3& pos) {
    getBodyTable()->getPos()[index] = pos;
}

void Rigid::setVelocity(const glm::vec3& vel) {
    getBodyTable()->getVel()[index] = vel;
}

glm::vec3& Rigid::getPos() { return getBodyTable()->getPos()[index]; }
glm::vec3& Rigid::getInitial() { return getBodyTable()->getInitial()[index]; }
glm::vec3& Rigid::getInertial() { return getBodyTable()->getInertial()[index]; }
glm::vec3& Rigid::getVel() { return getBodyTable()->getVel()[index]; }
glm::vec3& Rigid::getPrevVel() { return getBodyTable()->getPrevVel()[index]; }
glm::vec2& Rigid::getScale() { return getBodyTable()->getScale()[index]; }
float& Rigid::getFriction() { return getBodyTable()->getFriction()[index]; }
float& Rigid::getMass() { return getBodyTable()->getMass()[index]; }
float& Rigid::getMoment() { return getBodyTable()->getMoment()[index]; }
float& Rigid::getRadius() { return getBodyTable()->getRadius()[index]; }
uint& Rigid::getCollider() { return getBodyTable()->getCollider()[index]; }
glm::mat2x2& Rigid::getMat() { return getBodyTable()->getMat()[index]; }
glm::mat2x2& Rigid::getIMat() { return getBodyTable()->getIMat()[index]; }
glm::mat2x2& Rigid::getRMat() { return getBodyTable()->getRMat()[index]; }
bool Rigid::getUpdated() { return getBodyTable()->getUpdated()[index]; }

}

