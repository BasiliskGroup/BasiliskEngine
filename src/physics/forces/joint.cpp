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

#include <basilisk/util/includes.h>
#include <basilisk/physics/forces/joint.h>
#include <basilisk/physics/rigid.h>
#include <basilisk/physics/solver.h>
#include <basilisk/physics/maths.h>

namespace bsk::internal {

Joint::Joint(Solver* solver, Rigid* bodyA, Rigid* bodyB, glm::vec2 rA, glm::vec2 rB, glm::vec3 stiffness, float fracture)
    : Force(solver, bodyA, bodyB), rA(rA), rB(rB)
{
    this->stiffness[0] = stiffness.x;
    this->stiffness[1] = stiffness.y;
    this->stiffness[2] = stiffness.z;
    this->fmax[2] = fracture;
    this->fmin[2] = -fracture;
    this->fracture[2] = fracture;
    this->restAngle = (bodyA ? bodyA->getPosition().z : 0.0f) - bodyB->getPosition().z;
    this->torqueArm = lengthSq((bodyA ? bodyA->getSize() : glm::vec2{ 0, 0 }) + bodyB->getSize());
}

bool Joint::initialize()
{
    // Store constraint function at beginnning of timestep C(x-)
    // Note: if bodyA is null, it is assumed that the joint connects a body to the world space position rA
    glm::vec2 c0xy = (bodyA ? transform(bodyA->getPosition(), rA) : rA) - transform(bodyB->getPosition(), rB);
    C0.x = c0xy.x;
    C0.y = c0xy.y;
    C0.z = ((bodyA ? bodyA->getPosition().z : 0) - bodyB->getPosition().z - restAngle) * torqueArm;
    return stiffness[0] != 0 || stiffness[1] != 0 || stiffness[2] != 0;
}

void Joint::computeConstraint(float alpha)
{
    // Compute constraint function at current state C(x)
    glm::vec3 Cn;
    glm::vec2 cnxy = (bodyA ? transform(bodyA->getPosition(), rA) : rA) - transform(bodyB->getPosition(), rB);
    Cn.x = cnxy.x;
    Cn.y = cnxy.y;
    Cn.z = ((bodyA ? bodyA->getPosition().z : 0) - bodyB->getPosition().z - restAngle) * torqueArm;

    for (int i = 0; i < rows(); i++)
    {
        // Store stabilized constraint function, if a hard constraint (Eq. 18)
        if (isinf(stiffness[i]))
            C[i] = Cn[i] - C0[i] * alpha;
        else
            C[i] = Cn[i];
    }
}

void Joint::computeDerivatives(Rigid* body)
{
    // Compute the first and second derivatives for the desired body
    if (body == bodyA)
    {
        glm::vec2 r = rotate(bodyA->getPosition().z, rA);
        J[0] = glm::vec3(1.0f, 0.0f, -r.y);
        J[1] = glm::vec3(0.0f, 1.0f, r.x);
        J[2] = glm::vec3(0.0f, 0.0f, torqueArm);
        // GLM 3x3 constructor: mat3(x1,y1,z1, x2,y2,z2, x3,y3,z3) = columns
        H[0] = glm::mat3(0, 0, 0, 0, 0, 0, -r.x, 0, 0);
        H[1] = glm::mat3(0, 0, 0, 0, 0, 0, -r.y, 0, 0);
        H[2] = glm::mat3(0, 0, 0, 0, 0, 0, 0, 0, 0);
    }
    else
    {
        glm::vec2 r = rotate(bodyB->getPosition().z, rB);
        J[0] = glm::vec3(-1.0f, 0.0f, r.y);
        J[1] = glm::vec3(0.0f, -1.0f, -r.x);
        J[2] = glm::vec3(0.0f, 0.0f, -torqueArm);
        H[0] = glm::mat3(0, 0, 0, 0, 0, 0, r.x, 0, 0);
        H[1] = glm::mat3(0, 0, 0, 0, 0, 0, r.y, 0, 0);
        H[2] = glm::mat3(0, 0, 0, 0, 0, 0, 0, 0, 0);
    }
}

}