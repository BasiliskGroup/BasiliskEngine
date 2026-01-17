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
    setStiffness(0, stiffness.x);
    setStiffness(1, stiffness.y);
    setStiffness(2, stiffness.z);
    setFmax(2, fracture);
    setFmin(2, -fracture);
    setFracture(2, fracture);
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
    return getStiffness(0) != 0 || getStiffness(1) != 0 || getStiffness(2) != 0;
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
        if (glm::isinf(getStiffness(i)))
            setC(i, Cn[i] - C0[i] * alpha);
        else
            setC(i, Cn[i]);
    }
}

void Joint::computeDerivatives(Rigid* body)
{
    // Compute the first and second derivatives for the desired body
    if (body == bodyA)
    {
        glm::vec2 r = rotate(bodyA->getPosition().z, rA);
        setJ(0, bodyA, glm::vec3(1.0f, 0.0f, -r.y));
        setJ(1, bodyA, glm::vec3(0.0f, 1.0f, r.x));
        setJ(2, bodyA, glm::vec3(0.0f, 0.0f, torqueArm));
        setH(0, bodyA, glm::mat3(0, 0, 0, 0, 0, 0, -r.x, 0, 0));
        setH(1, bodyA, glm::mat3(0, 0, 0, 0, 0, 0, -r.y, 0, 0));
        setH(2, bodyA, glm::mat3(0, 0, 0, 0, 0, 0, 0, 0, 0));
    }
    else
    {
        glm::vec2 r = rotate(bodyB->getPosition().z, rB);
        setJ(0, bodyB, glm::vec3(-1.0f, 0.0f, r.y));
        setJ(1, bodyB, glm::vec3(0.0f, -1.0f, -r.x));
        setJ(2, bodyB, glm::vec3(0.0f, 0.0f, -torqueArm));
        setH(0, bodyB, glm::mat3(0, 0, 0, 0, 0, 0, r.x, 0, 0));
        setH(1, bodyB, glm::mat3(0, 0, 0, 0, 0, 0, r.y, 0, 0));
        setH(2, bodyB, glm::mat3(0, 0, 0, 0, 0, 0, 0, 0, 0));
    }
}

}