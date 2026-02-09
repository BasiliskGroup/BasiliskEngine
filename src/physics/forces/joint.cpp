#include <basilisk/util/includes.h>
#include <basilisk/physics/forces/joint.h>
#include <basilisk/physics/rigid.h>
#include <basilisk/physics/solver.h>
#include <basilisk/physics/maths.h>
#include <basilisk/physics/tables/forceTable.h>
#include <basilisk/physics/tables/forceTypeTable.h>

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

    // register to joint table
    solver->getForceTable()->getJointTable()->insert(this);
}

Joint::~Joint() {
    // unregister from joint table
    solver->getForceTable()->getJointTable()->markAsDeleted(this->specialIndex);
}

bool Joint::initialize() {
    // Store constraint function at beginnning of timestep C(x-)
    // Note: if bodyA is null, it is assumed that the joint connects a body to the world space position rA
    glm::vec2 c0xy = (bodyA ? transform(bodyA->getPosition(), getRA()) : getRA()) - transform(bodyB->getPosition(), getRB());
    setC0(glm::vec3(c0xy.x, c0xy.y, ((bodyA ? bodyA->getPosition().z : 0) - bodyB->getPosition().z - getRestAngle()) * getTorqueArm()));
    return getStiffness(0) != 0 || getStiffness(1) != 0 || getStiffness(2) != 0;
}

void Joint::computeConstraint(ForceTable* forceTable, std::size_t index, float alpha) {
    // Compute constraint function at current state C(x)
    JointStruct& joints = forceTable->getJoints(index);
    glm::vec3 Cn;
    glm::vec2 cnxy = transform(forceTable->getPosA(index), joints.rA) - transform(forceTable->getPosB(index), joints.rB);
    Cn.x = cnxy.x;
    Cn.y = cnxy.y;
    Cn.z = (forceTable->getPosA(index).z - forceTable->getPosB(index).z - joints.restAngle) * joints.torqueArm;

    for (int i = 0; i < 3; i++) {
        // Store stabilized constraint function, if a hard constraint (Eq. 18)
        if (glm::isinf(forceTable->getStiffness(index, i))) {
            forceTable->setC(index, i, Cn[i] - joints.C0[i] * alpha);
        } else {
            forceTable->setC(index, i, Cn[i]);
        }
    }
}

void Joint::computeDerivatives(ForceTable* forceTable, std::size_t index, ForceBodyOffset body) {
    JointStruct& joints = forceTable->getJoints(index);

    // Compute the first and second derivatives for the desired body
    if (body == ForceBodyOffset::A)
    {
        glm::vec2 r = rotate(forceTable->getPosA(index).z, joints.rA);
        forceTable->setJA(index, 0, glm::vec3(1.0f, 0.0f, -r.y));
        forceTable->setJA(index, 1, glm::vec3(0.0f, 1.0f, r.x));
        forceTable->setJA(index, 2, glm::vec3(0.0f, 0.0f, joints.torqueArm));
        forceTable->setHA(index, 0, glm::mat3(0, 0, 0, 0, 0, 0, -r.x, 0, 0));
        forceTable->setHA(index, 1, glm::mat3(0, 0, 0, 0, 0, 0, -r.y, 0, 0));
        forceTable->setHA(index, 2, glm::mat3(0, 0, 0, 0, 0, 0, 0, 0, 0));
    } else {
        glm::vec2 r = rotate(forceTable->getPosB(index).z, joints.rB);
        forceTable->setJB(index, 0, glm::vec3(-1.0f, 0.0f, r.y));
        forceTable->setJB(index, 1, glm::vec3(0.0f, -1.0f, -r.x));
        forceTable->setJB(index, 2, glm::vec3(0.0f, 0.0f, -joints.torqueArm));
        forceTable->setHB(index, 0, glm::mat3(0, 0, 0, 0, 0, 0, r.x, 0, 0));
        forceTable->setHB(index, 1, glm::mat3(0, 0, 0, 0, 0, 0, r.y, 0, 0));
        forceTable->setHB(index, 2, glm::mat3(0, 0, 0, 0, 0, 0, 0, 0, 0));
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