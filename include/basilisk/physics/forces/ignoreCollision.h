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

#include <basilisk/physics/forces/force.h>

namespace bsk::internal {

// Force which has no physical effect, but is used to ignore collisions between two bodies
class IgnoreCollision : public Force
{
public:
    IgnoreCollision(Solver* solver, Rigid* bodyA, Rigid* bodyB)
        : Force(solver, bodyA, bodyB) {}

    int rows() const override { return 0; }

    bool initialize() override { return true; }
    void computeConstraint(float alpha) override {}
    void computeDerivatives(Rigid* body) override {}
};

}

