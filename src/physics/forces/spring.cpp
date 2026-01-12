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
#include <basilisk/physics/forces/spring.h>
#include <basilisk/physics/rigid.h>
#include <basilisk/physics/solver.h>
#include <basilisk/physics/maths.h>

namespace bsk::internal {

Spring::Spring(Solver* solver, Rigid* bodyA, Rigid* bodyB, glm::vec2 rA, glm::vec2 rB, float stiffness, float rest)
    : Force(solver, bodyA, bodyB), rA(rA), rB(rB), rest(rest)
{
    this->stiffness[0] = stiffness;
    if (this->rest < 0)
        this->rest = length(transform(bodyA->getPosition(), rA) - transform(bodyB->getPosition(), rB));
}

void Spring::computeConstraint(float alpha)
{
    // Compute constraint function at current state C(x)
    C[0] = length(transform(bodyA->getPosition(), rA) - transform(bodyB->getPosition(), rB)) - rest;
}

void Spring::computeDerivatives(Rigid* body)
{
    // Compute the first and second derivatives for the desired body
    // GLM matrices are column-major: mat2(x1, y1, x2, y2) = columns (x1,x2), (y1,y2)
    glm::mat2 S = glm::mat2(0, 1, -1, 0);  // [0 -1; 1 0] -> columns (0,1), (-1,0)
    glm::mat2 I = glm::mat2(1, 0, 0, 1);   // Identity: columns (1,0), (0,1)

    glm::vec2 d = transform(bodyA->getPosition(), rA) - transform(bodyB->getPosition(), rB);
    float dlen2 = dot(d, d);
    if (dlen2 == 0)
        return;

    float dlen = sqrtf(dlen2);
    glm::vec2 n = d / dlen;
    glm::mat2 dxx = (I - outer(n, n)) / dlen;

    if (body == bodyA)
    {
        glm::vec2 Sr = rotate(bodyA->getPosition().z, S * rA);
        glm::vec2 r = rotate(bodyA->getPosition().z, rA);

        glm::vec2 dxr = dxx * Sr;
        float drr = -dot(n, r) - dot(n, r);

        JA[0].x = n.x;
        JA[0].y = n.y;
        JA[0].z = dot(n, Sr);
        // GLM 3x3 constructor: mat3(x1,y1,z1, x2,y2,z2, x3,y3,z3) = columns
        // GLM matrices are column-major: mat[col][row]
        glm::vec2 row0 = glm::vec2(dxx[0][0], dxx[1][0]);  // row 0
        glm::vec2 row1 = glm::vec2(dxx[0][1], dxx[1][1]);  // row 1
        HA[0] = glm::mat3(
            row0.x, row1.x, dxr.x,   // column 0
            row0.y, row1.y, dxr.y,   // column 1
            dxr.x,  dxr.y,  drr      // column 2
        );
    }
    else
    {
        glm::vec2 Sr = rotate(bodyB->getPosition().z, S * rB);
        glm::vec2 r = rotate(bodyB->getPosition().z, rB);
        glm::vec2 dxr = dxx * Sr;
        float drr = dot(n, r) + dot(n, r);

        JB[0].x = -n.x;
        JB[0].y = -n.y;
        JB[0].z = dot(n, -Sr);
        glm::vec2 row0 = glm::vec2(dxx[0][0], dxx[1][0]);  // row 0
        glm::vec2 row1 = glm::vec2(dxx[0][1], dxx[1][1]);  // row 1
        HB[0] = glm::mat3(
            row0.x, row1.x, dxr.x,   // column 0
            row0.y, row1.y, dxr.y,   // column 1
            dxr.x,  dxr.y,  drr      // column 2
        );
    }
}

}