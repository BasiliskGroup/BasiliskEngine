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

namespace bsk::internal {

Force::Force(Solver* solver, Rigid* bodyA, Rigid* bodyB)
    : solver(solver), bodyA(bodyA), bodyB(bodyB), next(nullptr), nextA(nullptr), nextB(nullptr), prev(nullptr), prevA(nullptr), prevB(nullptr)
{
    // Add to solver linked list
    solver->insert(this);

    // Add to body linked lists
    if (bodyA)
    {
        bodyA->insert(this);
    }
    if (bodyB)
    {
        bodyB->insert(this);
    }

    // Set some reasonable defaults
    for (int i = 0; i < MAX_ROWS; i++)
    {
        JA[i] = { 0, 0, 0 };
        JB[i] = { 0, 0, 0 };
        HA[i] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
        HB[i] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
        C[i] = 0.0f;
        stiffness[i] = INFINITY;
        fmax[i] = INFINITY;
        fmin[i] = -INFINITY;
        fracture[i] = INFINITY;

        penalty[i] = 0.0f;
        lambda[i] = 0.0f;
    }
}


Force::~Force()
{
    // Remove from solver linked list
    solver->remove(this);

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

void Force::disable()
{
    // Disable this force by clearing the relavent fields
    for (int i = 0; i < MAX_ROWS; i++)
    {
        stiffness[i] = 0;
        penalty[i] = 0;
        lambda[i] = 0;
    }
}

}