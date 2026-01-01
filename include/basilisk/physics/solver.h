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

#pragma once

#include <basilisk/physics/rigid.h>
#include <basilisk/physics/force.h>

#define PENALTY_MIN 1.0f              // Minimum penalty parameter
#define PENALTY_MAX 1000000000.0f     // Maximum penalty parameter
#define COLLISION_MARGIN 0.0005f      // Margin for collision detection to avoid flickering contacts
#define STICK_THRESH 0.01f            // Position threshold for sticking contacts (ie static friction)

namespace bsk::internal {
    class ColliderTable;
}

namespace bsk::internal {

// Core solver class which holds all the rigid bodies and forces, and has logic to step the simulation forward in time
struct Solver
{
    float gravity;      // Gravity
    int iterations;     // Solver iterations
    float dt;           // Timestep

    float alpha;        // Stabilization parameter
    float beta;         // Penalty ramping parameter
    float gamma;        // Warmstarting decay parameter

    bool postStabilize; // Whether to apply post-stabilization to the system

    Rigid* bodies;
    Force* forces;

    ColliderTable* colliderTable;

    Solver();
    ~Solver();

    // Linked list management
    void insert(Rigid* body);
    void remove(Rigid* body);
    void insert(Force* force);
    void remove(Force* force);

    Rigid* pick(glm::vec2 at, glm::vec2& local);
    void clear();
    void defaultParams();
    void step(float dt);

    ColliderTable* getColliderTable() const { return colliderTable; }
};

}
