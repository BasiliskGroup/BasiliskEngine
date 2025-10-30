#include "solver/physics.h"


Rigid::Rigid(Solver* solver, Node2D* node, vec3 pos, vec2 scale, float density, float friction, vec3 vel, Collider* collider) : 
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

    index = solver->getBodyTable()->insert(this, pos, vel, scale, friction, mass, collider->getIndex(), radius);
}   

Rigid::Rigid(Solver* solver, Node2D* node, vec3 pos, vec2 scale, float density, float friction, vec3 vel, uint collider) : 
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

    index = solver->getBodyTable()->insert(this, pos, vel, scale, friction, mass, collider, radius);
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

vec3 Rigid::getPos() {
    return getBodyTable()->getPos()[index];
}

vec2 Rigid::getScale() {
    return getBodyTable()->getScale()[index];
}

float Rigid::getDensity() {
    return density;
}

float Rigid::getFriction() {
    return getBodyTable()->getFriction()[index];
}

vec3 Rigid::getVel() {
    return getBodyTable()->getVel()[index];
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

        relations.emplace_back(other->getIndex(), f->getType());
    }
}

// ----------------------
// Broad Collision
// ----------------------

ushort Rigid::constrainedTo(uint other) const {
    // check if this body is constrained to the other body
    for (const auto& rel : relations) {
        if (rel.first == other) {
            return rel.second;
        }
    }
    return -1;
}

void Rigid::draw() {

}

void Rigid::clear() {
    if (node != nullptr) {
        delete node;
        node = nullptr;
    }
}