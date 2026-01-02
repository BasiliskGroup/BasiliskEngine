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

#include <basilisk/physics/rigid.h>
#include <basilisk/physics/solver.h>
#include <basilisk/nodes/node2d.h>
#include <basilisk/physics/tables/bodyTable.h>

namespace bsk::internal {

Rigid::Rigid(Solver* solver, Node2D* node, Collider* collider, glm::vec3 position, glm::vec2 size, float density, float friction, glm::vec3 velocity)
    : solver(solver), node(node), forces(nullptr), next(nullptr), prev(nullptr), collider(collider) {
    // Add to linked list
    solver->insert(this);
    this->solver->getBodyTable()->insert(this, position, size, density, friction, velocity, collider);
}


Rigid::~Rigid() {
    // Remove from linked list
    solver->remove(this);

    // Delete all forces
    Force* curForce = forces;
    while (curForce) {
        Force* nextForce = (curForce->bodyA == this) ? curForce->nextA : curForce->nextB;
        delete curForce;
        curForce = nextForce;
    }
}

bool Rigid::constrainedTo(Rigid* other) const {
    // Check if this body is constrained to the other body
    for (Force* f = forces; f != nullptr; f = (f->bodyA == this) ? f->nextA : f->nextB)
        if ((f->bodyA == this && f->bodyB == other) || (f->bodyA == other && f->bodyB == this))
            return true;
    return false;
}

void Rigid::insert(Force* force) {
    if (force == nullptr) {
        return;
    }

    // Determine if this body is bodyA or bodyB
    if (force->bodyA == this) {
        // This is bodyA
        force->nextA = forces;
        force->prevA = nullptr;

        if (forces) {
            // Update the prev pointer of the old head
            if (forces->bodyA == this) {
                forces->prevA = force;
            } else {
                forces->prevB = force;
            }
        }
    } else {
        // This is bodyB
        force->nextB = forces;
        force->prevB = nullptr;

        if (forces) {
            // Update the prev pointer of the old head
            if (forces->bodyA == this) {
                forces->prevA = force;
            } else {
                forces->prevB = force;
            }
        }
    }

    forces = force;
}

void Rigid::remove(Force* force) {
    if (force == nullptr) {
        return;
    }

    // Determine if this body is bodyA or bodyB
    bool isBodyA = (force->bodyA == this);

    Force* prev = isBodyA ? force->prevA : force->prevB;
    Force* next = isBodyA ? force->nextA : force->nextB;

    if (prev) {
        // Update prev's next pointer
        if (prev->bodyA == this) {
            prev->nextA = next;
        } else {
            prev->nextB = next;
        }
    } else {
        // This was the head of the list
        forces = next;
    }

    if (next) {
        // Update next's prev pointer
        if (next->bodyA == this) {
            next->prevA = prev;
        } else {
            next->prevB = prev;
        }
    }

    // Clear this force's pointers
    if (isBodyA) {
        force->prevA = nullptr;
        force->nextA = nullptr;
    } else {
        force->prevB = nullptr;
        force->nextB = nullptr;
    }
}

void Rigid::setPosition(const glm::vec3& pos) {
    this->solver->getBodyTable()->setPos(this->index, pos);
}

void Rigid::setScale(const glm::vec2& scale) {
    this->solver->getBodyTable()->setScale(this->index, scale);
}

void Rigid::setVelocity(const glm::vec3& vel) {
    this->solver->getBodyTable()->setVel(this->index, vel);
}

glm::vec3 Rigid::getVelocity() const {
    return this->solver->getBodyTable()->getVel(this->index);
}

float Rigid::getDensity() const {
    float mass = this->solver->getBodyTable()->getMass(this->index);
    glm::vec2 size = this->solver->getBodyTable()->getScale(this->index);
    return mass / (size.x * size.y);
}

float Rigid::getFriction() const {
    return this->solver->getBodyTable()->getFriction(this->index);
}

glm::vec3 Rigid::getVel() const {
    return this->solver->getBodyTable()->getVel(this->index);
}

void Rigid::setInitial(const glm::vec3& initial) {
    this->solver->getBodyTable()->setInitial(this->index, initial);
}

void Rigid::setInertial(const glm::vec3& inertial) {
    this->solver->getBodyTable()->setInertial(this->index, inertial);
}

void Rigid::setPrevVelocity(const glm::vec3& prevVelocity) {
    this->solver->getBodyTable()->setPrevVel(this->index, prevVelocity);
}

void Rigid::setMass(float mass) {
    this->solver->getBodyTable()->setMass(this->index, mass);
}

void Rigid::setMoment(float moment) {
    this->solver->getBodyTable()->setMoment(this->index, moment);
}

void Rigid::setFriction(float friction) {
    this->solver->getBodyTable()->setFriction(this->index, friction);
}

void Rigid::setRadius(float radius) {
    this->solver->getBodyTable()->setRadius(this->index, radius);
}

void Rigid::setCollider(Collider* collider) {
    this->collider = collider;
}

void Rigid::setForces(Force* forces) {
    this->forces = forces;
}

void Rigid::setNext(Rigid* next) {
    this->next = next;
}

void Rigid::setPrev(Rigid* prev) {
    this->prev = prev;
}

void Rigid::setNode(Node2D* node) {
    this->node = node;
}

void Rigid::setIndex(std::size_t index) {
    this->index = index;
}

glm::vec3 Rigid::getPosition() const {
    return this->solver->getBodyTable()->getPos(this->index);
}

glm::vec3 Rigid::getInitial() const {
    return this->solver->getBodyTable()->getInitial(this->index);
}

glm::vec3 Rigid::getInertial() const {
    return this->solver->getBodyTable()->getInertial(this->index);
}

glm::vec3 Rigid::getPrevVelocity() const {
    return this->solver->getBodyTable()->getPrevVel(this->index);
}

glm::vec2 Rigid::getSize() const {
    return this->solver->getBodyTable()->getScale(this->index);
}

float Rigid::getMass() const {
    return this->solver->getBodyTable()->getMass(this->index);
}

float Rigid::getMoment() const {
    return this->solver->getBodyTable()->getMoment(this->index);
}

float Rigid::getRadius() const {
    return this->solver->getBodyTable()->getRadius(this->index);
}

}