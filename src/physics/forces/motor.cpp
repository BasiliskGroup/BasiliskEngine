#include <basilisk/physics/forces/motor.h>
#include <basilisk/physics/rigid.h>
#include <basilisk/physics/solver.h>    
#include <basilisk/physics/tables/forceTable.h>
#include <basilisk/physics/tables/forceTypeTable.h>

namespace bsk::internal {

Motor::Motor(Solver* solver, Rigid* bodyA, Rigid* bodyB, float speed, float maxTorque)
    : Force(solver, bodyA, bodyB)
{
    setSpeed(speed);
    setFmax(0, maxTorque);
    setFmin(0, -maxTorque);
    solver->getForceTable()->setForceType(this->index, ForceType::MOTOR);

    // register to motor table
    solver->getForceTable()->getMotorTable()->insert(this);
}

Motor::~Motor() {
    // unregister from motor table
    solver->getForceTable()->getMotorTable()->markAsDeleted(this->specialIndex);
}

void Motor::computeConstraint(ForceTable* forceTable, std::size_t specialIndex, float alpha) {
    std::size_t index = forceTable->getMotorTable()->getForceIndex(specialIndex);
    MotorStruct& motors = forceTable->getMotorTable()->getData(specialIndex);

    // Compute delta angular position between the two bodies
    float dAngleA = forceTable->getPosA(index).z - forceTable->getInitialA(index).z;
    float dAngleB = forceTable->getPosB(index).z - forceTable->getInitialB(index).z;
    float deltaAngle = dAngleA - dAngleB;

    // Constraint tries to reach desired angular speed
    forceTable->setC(index, 0, deltaAngle - motors.speed * forceTable->getSolver()->getDt());
}

void Motor::computeDerivatives(ForceTable* forceTable, std::size_t specialIndex, uint32_t bodyIndex, const glm::vec3& jacobianMask) {
    std::size_t index = forceTable->getMotorTable()->getForceIndex(specialIndex);
    // Compute the first and second derivatives for the desired body
    if (bodyIndex == forceTable->getBodies(index).a) {
        forceTable->setJ(index, 0, glm::vec3(0.0f, 0.0f, 1.0f) * jacobianMask);
        forceTable->setH(index, 0, glm::mat3(0, 0, 0, 0, 0, 0, 0, 0, 0));
    } else {
        forceTable->setJ(index, 0, glm::vec3(0.0f, 0.0f, -1.0f) * jacobianMask);
        forceTable->setH(index, 0, glm::mat3(0, 0, 0, 0, 0, 0, 0, 0, 0));
    }
}

// Getters
MotorStruct& Motor::getData() { return solver->getForceTable()->getMotorTable()->getData(specialIndex); }
const MotorStruct& Motor::getData() const { return solver->getForceTable()->getMotorTable()->getData(specialIndex); }
float Motor::getSpeed() const { return getData().speed; }

// Setters
void Motor::setData(const MotorStruct& value) { getData() = value; }
void Motor::setSpeed(float value) { getData().speed = value; }

}