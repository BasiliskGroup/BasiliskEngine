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
struct JointStruct {
    glm::vec2 rA = glm::vec2(0.0f);
    glm::vec2 rB = glm::vec2(0.0f);
    glm::vec3 C0 = glm::vec3(0.0f);
    float torqueArm = 0.0f;
    float restAngle = 0.0f;
};

// Revolute joint + angle constraint between two rigid bodies, with optional fracture
class Joint : public Force {
public:
    Joint(Solver* solver, Rigid* bodyA, Rigid* bodyB, glm::vec2 rA, glm::vec2 rB, glm::vec3 stiffness = glm::vec3{ INFINITY, INFINITY, INFINITY },
        float fracture = INFINITY);

    int rows() const override { return 3; }

    bool initialize() override;
    void computeConstraint(float alpha) override;
    void computeDerivatives(Rigid* body) override;
    
    // Getters
    glm::vec2 getRA() const;
    glm::vec2 getRB() const;
    glm::vec3 getC0() const;
    float getTorqueArm() const;
    float getRestAngle() const;
    JointStruct& getData();
    const JointStruct& getData() const;
    
    // Setters
    void setRA(const glm::vec2& value);
    void setRB(const glm::vec2& value);
    void setC0(const glm::vec3& value);
    void setTorqueArm(float value);
    void setRestAngle(float value);
    
    // Mutable references for direct access (for performance-critical code)
    glm::vec2& getRARef();
    glm::vec2& getRBRef();
    glm::vec3& getC0Ref();
    void setData(const JointStruct& value);
};

}

