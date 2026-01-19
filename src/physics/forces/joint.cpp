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
#include <basilisk/physics/tables/forceTable.h>

namespace bsk::internal {

Joint::Joint(Solver* solver, Rigid* bodyA, Rigid* bodyB, glm::vec2 rA, glm::vec2 rB, glm::vec3 stiffness, float fracture)
    : Force(solver, bodyA, bodyB)
{
    setRA(rA);
    setRB(rB);

    setStiffness(0, stiffness.x);
    setStiffness(1, stiffness.y);
    setStiffness(2, stiffness.z);
    setFmax(2, fracture);
    setFmin(2, -fracture);
    setFracture(2, fracture);
    setRestAngle((bodyA ? bodyA->getPosition().z : 0.0f) - bodyB->getPosition().z);
    setTorqueArm(lengthSq((bodyA ? bodyA->getSize() : glm::vec2{ 0, 0 }) + bodyB->getSize()));
    solver->getForceTable()->setForceType(this->index, ForceType::JOINT);
}

bool Joint::initialize()
{
    // Store constraint function at beginnning of timestep C(x-)
    // Note: if bodyA is null, it is assumed that the joint connects a body to the world space position rA
    glm::vec2 c0xy = (bodyA ? transform(bodyA->getPosition(), getRA()) : getRA()) - transform(bodyB->getPosition(), getRB());
    setC0(glm::vec3(c0xy.x, c0xy.y, ((bodyA ? bodyA->getPosition().z : 0) - bodyB->getPosition().z - getRestAngle()) * getTorqueArm()));
    return getStiffness(0) != 0 || getStiffness(1) != 0 || getStiffness(2) != 0;
}

void Joint::computeConstraint(float alpha)
{
    // Compute constraint function at current state C(x)
    glm::vec3 Cn;
    glm::vec2 cnxy = (bodyA ? transform(bodyA->getPosition(), getRA()) : getRA()) - transform(bodyB->getPosition(), getRB());
    Cn.x = cnxy.x;
    Cn.y = cnxy.y;
    Cn.z = ((bodyA ? bodyA->getPosition().z : 0) - bodyB->getPosition().z - getRestAngle()) * getTorqueArm();

    for (int i = 0; i < rows(); i++)
    {
        // Store stabilized constraint function, if a hard constraint (Eq. 18)
        if (glm::isinf(getStiffness(i)))
            setC(i, Cn[i] - getC0()[i] * alpha);
        else
            setC(i, Cn[i]);
    }
}

void Joint::computeDerivatives(Rigid* body)
{
    // Compute the first and second derivatives for the desired body
    if (body == bodyA)
    {
        glm::vec2 r = rotate(bodyA->getPosition().z, getRA());
        setJ(0, bodyA, glm::vec3(1.0f, 0.0f, -r.y));
        setJ(1, bodyA, glm::vec3(0.0f, 1.0f, r.x));
        setJ(2, bodyA, glm::vec3(0.0f, 0.0f, getTorqueArm()));
        setH(0, bodyA, glm::mat3(0, 0, 0, 0, 0, 0, -r.x, 0, 0));
        setH(1, bodyA, glm::mat3(0, 0, 0, 0, 0, 0, -r.y, 0, 0));
        setH(2, bodyA, glm::mat3(0, 0, 0, 0, 0, 0, 0, 0, 0));
    }
    else
    {
        glm::vec2 r = rotate(bodyB->getPosition().z, getRB());
        setJ(0, bodyB, glm::vec3(-1.0f, 0.0f, r.y));
        setJ(1, bodyB, glm::vec3(0.0f, -1.0f, -r.x));
        setJ(2, bodyB, glm::vec3(0.0f, 0.0f, -getTorqueArm()));
        setH(0, bodyB, glm::mat3(0, 0, 0, 0, 0, 0, r.x, 0, 0));
        setH(1, bodyB, glm::mat3(0, 0, 0, 0, 0, 0, r.y, 0, 0));
        setH(2, bodyB, glm::mat3(0, 0, 0, 0, 0, 0, 0, 0, 0));
    }
}

// Getters
JointStruct& Joint::getData() { return solver->getForceTable()->getJoints(index); }
const JointStruct& Joint::getData() const { return solver->getForceTable()->getJoints(index); }
glm::vec2 Joint::getRA() const { return getData().rA; }
glm::vec2 Joint::getRB() const { return getData().rB; }
glm::vec3 Joint::getC0() const { return getData().C0; }
float Joint::getTorqueArm() const { return getData().torqueArm; }
float Joint::getRestAngle() const { return getData().restAngle; }

// Setters
void Joint::setData(const JointStruct& value) { getData() = value; }
void Joint::setRA(const glm::vec2& value) { getData().rA = value; }
void Joint::setRB(const glm::vec2& value) { getData().rB = value; }
void Joint::setC0(const glm::vec3& value) { getData().C0 = value; }
void Joint::setTorqueArm(float value) { getData().torqueArm = value; }
void Joint::setRestAngle(float value) { getData().restAngle = value; }

// Mutable references for direct access (for performance-critical code)
glm::vec2& Joint::getRARef() { return getData().rA; }
glm::vec2& Joint::getRBRef() { return getData().rB; }
glm::vec3& Joint::getC0Ref() { return getData().C0; }

}