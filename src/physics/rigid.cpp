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

namespace bsk::internal {

Rigid::Rigid(Solver* solver, Node2D* node, Collider* collider, glm::vec3 position, glm::vec2 size, float density, float friction, glm::vec3 velocity)
    : solver(solver), node(node), forces(nullptr), next(nullptr), prev(nullptr), position(position), velocity(velocity), prevVelocity(velocity), size(size), friction(friction)
{
    // Add to linked list
    solver->insert(this);

    // add collider
    this->collider = collider;

    // Compute mass properties and bounding radius
    mass = size.x * size.y * density;
    moment = mass * dot(size, size) / 12.0f;
    radius = length(size * 0.5f);
}


Rigid::~Rigid()
{
    // Remove from linked list
    solver->remove(this);

    // Delete all forces
    Force* curForce = forces;
    while (curForce)
    {
        Force* nextForce = (curForce->bodyA == this) ? curForce->nextA : curForce->nextB;
        delete curForce;
        curForce = nextForce;
    }
}

bool Rigid::constrainedTo(Rigid* other) const
{
    // Check if this body is constrained to the other body
    for (Force* f = forces; f != nullptr; f = (f->bodyA == this) ? f->nextA : f->nextB)
        if ((f->bodyA == this && f->bodyB == other) || (f->bodyA == other && f->bodyB == this))
            return true;
    return false;
}

void Rigid::insert(Force* force)
{
    if (force == nullptr)
    {
        return;
    }

    // Determine if this body is bodyA or bodyB
    if (force->bodyA == this)
    {
        // This is bodyA
        force->nextA = forces;
        force->prevA = nullptr;

        if (forces)
        {
            // Update the prev pointer of the old head
            if (forces->bodyA == this)
            {
                forces->prevA = force;
            }
            else
            {
                forces->prevB = force;
            }
        }
    }
    else
    {
        // This is bodyB
        force->nextB = forces;
        force->prevB = nullptr;

        if (forces)
        {
            // Update the prev pointer of the old head
            if (forces->bodyA == this)
            {
                forces->prevA = force;
            }
            else
            {
                forces->prevB = force;
            }
        }
    }

    forces = force;
}

void Rigid::remove(Force* force)
{
    if (force == nullptr)
    {
        return;
    }

    // Determine if this body is bodyA or bodyB
    bool isBodyA = (force->bodyA == this);

    Force* prev = isBodyA ? force->prevA : force->prevB;
    Force* next = isBodyA ? force->nextA : force->nextB;

    if (prev)
    {
        // Update prev's next pointer
        if (prev->bodyA == this)
        {
            prev->nextA = next;
        }
        else
        {
            prev->nextB = next;
        }
    }
    else
    {
        // This was the head of the list
        forces = next;
    }

    if (next)
    {
        // Update next's prev pointer
        if (next->bodyA == this)
        {
            next->prevA = prev;
        }
        else
        {
            next->prevB = prev;
        }
    }

    // Clear this force's pointers
    if (isBodyA)
    {
        force->prevA = nullptr;
        force->nextA = nullptr;
    }
    else
    {
        force->prevB = nullptr;
        force->nextB = nullptr;
    }
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