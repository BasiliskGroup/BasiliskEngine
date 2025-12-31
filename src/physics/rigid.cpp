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

#include <basilisk/physics/solver.h>
#include <basilisk/nodes/node2d.h>

namespace bsk::internal {

Rigid::Rigid(Solver* solver, Node2D* node, glm::vec3 position, glm::vec2 size, float density, float friction, glm::vec3 velocity)
    : solver(solver), node(node), forces(0), next(0), position(position), velocity(velocity), prevVelocity(velocity), size(size), friction(friction)
{
    // Add to linked list
    next = solver->bodies;
    solver->bodies = this;

    // Compute mass properties and bounding radius
    mass = size.x * size.y * density;
    moment = mass * dot(size, size) / 12.0f;
    radius = length(size * 0.5f);
}


Rigid::~Rigid()
{
    // Remove from linked list
    Rigid** p = &solver->bodies;
    while (*p != this)
        p = &(*p)->next;
    *p = next;
}

bool Rigid::constrainedTo(Rigid* other) const
{
    // Check if this body is constrained to the other body
    for (Force* f = forces; f != 0; f = f->next)
        if ((f->bodyA == this && f->bodyB == other) || (f->bodyA == other && f->bodyB == this))
            return true;
    return false;
}

void Rigid::setPosition(const glm::vec3& pos)
{
    position = pos;
}

void Rigid::setScale(const glm::vec2& scale)
{
    size = scale;
}

void Rigid::setVelocity(const glm::vec3& vel)
{
    velocity = vel;
}

glm::vec3 Rigid::getVelocity() const
{
    return velocity;
}

float Rigid::getDensity() const
{
    return mass / (size.x * size.y);
}

float Rigid::getFriction() const
{
    return friction;
}

glm::vec3 Rigid::getVel() const
{
    return velocity;
}

Solver* Rigid::getSolver() const
{
    return solver;
}

void Rigid::setNode(void* node)
{
    // Node pointer storage - not used in physics but needed for compatibility
    // The node pointer is not stored in the Rigid struct in the new implementation
}

}