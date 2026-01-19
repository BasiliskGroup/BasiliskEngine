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

#include <basilisk/physics/forces/motor.h>
#include <basilisk/physics/rigid.h>
#include <basilisk/physics/solver.h>    
#include <basilisk/physics/tables/forceTable.h>

namespace bsk::internal {

Motor::Motor(Solver* solver, Rigid* bodyA, Rigid* bodyB, float speed, float maxTorque)
    : Force(solver, bodyA, bodyB)
{
    setSpeed(speed);
    setFmax(0, maxTorque);
    setFmin(0, -maxTorque);
    solver->getForceTable()->setForceType(this->index, ForceType::MOTOR);
}

void Motor::computeConstraint(float alpha)
{
    // Compute delta angular position between the two bodies
    float dAngleA = (bodyA ? (bodyA->getPosition().z - bodyA->getInitial().z) : 0.0f);
    float dAngleB = bodyB->getPosition().z - bodyB->getInitial().z;
    float deltaAngle = dAngleA - dAngleB;

    // Constraint tries to reach desired angular speed
    setC(0, deltaAngle - getSpeed() * solver->getDt());
}

void Motor::computeDerivatives(Rigid* body)
{
    // Compute the first and second derivatives for the desired body
    if (body == bodyA)
    {
        setJ(0, bodyA, glm::vec3(0.0f, 0.0f, 1.0f));
        setH(0, bodyA, glm::mat3(0, 0, 0, 0, 0, 0, 0, 0, 0));
    }
    else
    {
        setJ(0, bodyB, glm::vec3(0.0f, 0.0f, -1.0f));
        setH(0, bodyB, glm::mat3(0, 0, 0, 0, 0, 0, 0, 0, 0));
    }
}

// Getters
MotorStruct& Motor::getData() { return solver->getForceTable()->getMotors(index); }
const MotorStruct& Motor::getData() const { return solver->getForceTable()->getMotors(index); }
float Motor::getSpeed() const { return getData().speed; }

// Setters
void Motor::setData(const MotorStruct& value) { getData() = value; }
void Motor::setSpeed(float value) { getData().speed = value; }

}