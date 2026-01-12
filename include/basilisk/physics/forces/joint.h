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

// Revolute joint + angle constraint between two rigid bodies, with optional fracture
class Joint : public Force
{
private:
    glm::vec2 rA, rB;
    glm::vec3 C0;
    float torqueArm;
    float restAngle;

public:
    Joint(Solver* solver, Rigid* bodyA, Rigid* bodyB, glm::vec2 rA, glm::vec2 rB, glm::vec3 stiffness = glm::vec3{ INFINITY, INFINITY, INFINITY },
        float fracture = INFINITY);

    int rows() const override { return 3; }

    bool initialize() override;
    void computeConstraint(float alpha) override;
    void computeDerivatives(Rigid* body) override;
    
    // Getters
    glm::vec2 getRA() const { return rA; }
    glm::vec2 getRB() const { return rB; }
    glm::vec3 getC0() const { return C0; }
    float getTorqueArm() const { return torqueArm; }
    float getRestAngle() const { return restAngle; }
    
    // Setters
    void setRA(const glm::vec2& value) { rA = value; }
    void setRB(const glm::vec2& value) { rB = value; }
    void setC0(const glm::vec3& value) { C0 = value; }
    void setTorqueArm(float value) { torqueArm = value; }
    void setRestAngle(float value) { restAngle = value; }
    
    // Mutable references for direct access (for performance-critical code)
    glm::vec2& getRARef() { return rA; }
    glm::vec2& getRBRef() { return rB; }
    glm::vec3& getC0Ref() { return C0; }
};

}

