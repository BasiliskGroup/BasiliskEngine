#include <basilisk/util/includes.h>
#include <basilisk/physics/forces/spring.h>
#include <basilisk/physics/rigid.h>
#include <basilisk/physics/solver.h>
#include <basilisk/physics/maths.h>
#include <basilisk/physics/tables/forceTable.h>
#include <basilisk/physics/tables/forceTypeTable.h>

namespace bsk::internal {

Spring::Spring(Solver* solver, Rigid* bodyA, Rigid* bodyB, glm::vec2 rA, glm::vec2 rB, float stiffness, float rest)
    : Force(solver, bodyA, bodyB)
{
    setRA(rA);
    setRB(rB);
    setRest(rest);
    setStiffness(0, stiffness);
    if (getRest() < 0)
        setRest(length(transform(bodyA->getPosition(), getRA()) - transform(bodyB->getPosition(), getRB())));
    solver->getForceTable()->setForceType(this->index, ForceType::SPRING);

    // register to spring table
    solver->getForceTable()->getSpringTable()->insert(this);
}

Spring::~Spring() {
    // unregister from spring table
    solver->getForceTable()->getSpringTable()->markAsDeleted(this->index);
}

void Spring::computeConstraint(ForceTable* forceTable, std::size_t index, float alpha) {
    // Compute constraint function at current state C(x)
    SpringStruct& springs = forceTable->getSprings(index);
    forceTable->setC(index, 0, length(transform(forceTable->getPosA(index), springs.rA) - transform(forceTable->getPosB(index), springs.rB)) - springs.rest);
}

void Spring::computeDerivatives(ForceTable* forceTable, std::size_t index, ForceBodyOffset body) {
    SpringStruct& springs = forceTable->getSprings(index);

    // Compute the first and second derivatives for the desired body
    // GLM matrices are column-major: mat2(x1, y1, x2, y2) = columns (x1,x2), (y1,y2)
    glm::mat2 S = glm::mat2(0, 1, -1, 0);  // [0 -1; 1 0] -> columns (0,1), (-1,0)
    glm::mat2 I = glm::mat2(1, 0, 0, 1);   // Identity: columns (1,0), (0,1)

    glm::vec2 d = transform(forceTable->getPosA(index), springs.rA) - transform(forceTable->getPosB(index), springs.rB);
    float dlen2 = dot(d, d);
    if (dlen2 == 0)
        return;

    float dlen = sqrtf(dlen2);
    glm::vec2 n = d / dlen;
    glm::mat2 dxx = (I - outer(n, n)) / dlen;

    if (body == ForceBodyOffset::A)
    {
        glm::vec2 Sr = rotate(forceTable->getPosA(index).z, S * springs.rA);
        glm::vec2 r = rotate(forceTable->getPosA(index).z, springs.rA);

        glm::vec2 dxr = dxx * Sr;
        float drr = -dot(n, r) - dot(n, r);

        forceTable->setJA(index, 0, glm::vec3(n.x, n.y, dot(n, Sr)));
        // GLM 3x3 constructor: mat3(x1,y1,z1, x2,y2,z2, x3,y3,z3) = columns
        // GLM matrices are column-major: mat[col][row]
        glm::vec2 row0 = glm::vec2(dxx[0][0], dxx[1][0]);  // row 0
        glm::vec2 row1 = glm::vec2(dxx[0][1], dxx[1][1]);  // row 1
        forceTable->setHA(index, 0, glm::mat3(
            row0.x, row1.x, dxr.x,   // column 0
            row0.y, row1.y, dxr.y,   // column 1
            dxr.x,  dxr.y,  drr      // column 2
        ));
    }
    else
    {
        glm::vec2 Sr = rotate(forceTable->getPosB(index).z, S * springs.rB);
        glm::vec2 r = rotate(forceTable->getPosB(index).z, springs.rB);
        glm::vec2 dxr = dxx * Sr;
        float drr = dot(n, r) + dot(n, r);

        forceTable->setJB(index, 0, glm::vec3(-n.x, -n.y, dot(n, -Sr)));
        glm::vec2 row0 = glm::vec2(dxx[0][0], dxx[1][0]);  // row 0
        glm::vec2 row1 = glm::vec2(dxx[0][1], dxx[1][1]);  // row 1
        forceTable->setHB(index, 0, glm::mat3(
            row0.x, row1.x, dxr.x,   // column 0
            row0.y, row1.y, dxr.y,   // column 1
            dxr.x,  dxr.y,  drr      // column 2
        ));
    }
}

// Getters
SpringStruct& Spring::getData() { return solver->getForceTable()->getSprings(index); }
const SpringStruct& Spring::getData() const { return solver->getForceTable()->getSprings(index); }
glm::vec2 Spring::getRA() const { return getData().rA; }
glm::vec2 Spring::getRB() const { return getData().rB; }
float Spring::getRest() const { return getData().rest; }

// Setters
void Spring::setData(const SpringStruct& value) { getData() = value; }
void Spring::setRest(float value) { getData().rest = value; }
void Spring::setRA(const glm::vec2& value) { getData().rA = value; }
void Spring::setRB(const glm::vec2& value) { getData().rB = value; }

// Mutable references for direct access (for performance-critical code)
glm::vec2& Spring::getRARef() { return getData().rA; }
glm::vec2& Spring::getRBRef() { return getData().rB; }
float& Spring::getRestRef() { return getData().rest; }

}