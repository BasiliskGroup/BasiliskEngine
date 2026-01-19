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

// ------------------------------------------------------------
// Table Struct
// ------------------------------------------------------------
struct MotorStruct {
    float speed = 0.0f;
};

// Motor force which applies a torque to two rigid bodies to achieve a desired angular speed
class Motor : public Force {
public:
    Motor(Solver* solver, Rigid* bodyA, Rigid* bodyB, float speed, float maxTorque);

    int rows() const override { return 1; }

    bool initialize() override { return true; }
    void computeConstraint(float alpha) override;
    void computeDerivatives(Rigid* body) override;
    
    // Getters
    float getSpeed() const;
    MotorStruct& getData();
    const MotorStruct& getData() const;
    
    // Setters
    void setSpeed(float value);
    void setData(const MotorStruct& value);
};

}

