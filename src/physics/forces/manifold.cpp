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
#include <basilisk/physics/forces/manifold.h>
#include <basilisk/physics/rigid.h>
#include <basilisk/physics/solver.h>
#include <basilisk/physics/maths.h>

namespace bsk::internal {

Manifold::Manifold(Solver* solver, Rigid* bodyA, Rigid* bodyB)
    : Force(solver, bodyA, bodyB), numContacts(0)
{
    setFmax(0, 0.0f);
    setFmax(2, 0.0f);
    setFmin(0, -INFINITY);
    setFmin(2, -INFINITY);
}

bool Manifold::initialize()
{
    // Compute friction
    friction = sqrtf(bodyA->getFriction() * bodyB->getFriction());

    // Store previous contact state
    Contact oldContacts[2] = { contacts[0], contacts[1] };
    float oldPenalty[4] = { getPenalty(0), getPenalty(1), getPenalty(2), getPenalty(3) };
    float oldLambda[4] = { getLambda(0), getLambda(1), getLambda(2), getLambda(3) };
    bool oldStick[2] = { contacts[0].stick, contacts[1].stick };
    int oldNumContacts = numContacts;

    // Compute new contacts
    numContacts = collide(bodyA, bodyB, contacts);

    // Merge old contact data with new contacts
    for (int i = 0; i < numContacts; i++)
    {
        setPenalty(i * 2 + 0, 0.0f);
        setPenalty(i * 2 + 1, 0.0f);
        setLambda(i * 2 + 0, 0.0f);
        setLambda(i * 2 + 1, 0.0f);

        for (int j = 0; j < oldNumContacts; j++)
        {
            if (contacts[i].feature.value == oldContacts[j].feature.value)
            {
                setPenalty(i * 2 + 0, oldPenalty[j * 2 + 0]);
                setPenalty(i * 2 + 1, oldPenalty[j * 2 + 1]);
                setLambda(i * 2 + 0, oldLambda[j * 2 + 0]);
                setLambda(i * 2 + 1, oldLambda[j * 2 + 1]);
                contacts[i].stick = oldStick[j];

                // If static friction in last frame, use the old contact points
                if (oldStick[j]) {
                    contacts[i].rA = oldContacts[j].rA;
                    contacts[i].rB = oldContacts[j].rB;
                }
            }
        }
    }

    for (int i = 0; i < numContacts; i++) {
        // Compute the contact basis (Eq. 15)
        glm::vec2 normal = contacts[i].normal;
        glm::vec2 tangent = { normal.y, -normal.x };
        // GLM 2x2 constructor: mat2(x1, y1, x2, y2) = columns (x1,x2), (y1,y2)
        glm::mat2 basis = glm::mat2(
            normal.x, tangent.x,   // column 0: (normal.x, tangent.x)
            normal.y, tangent.y    // column 1: (normal.y, tangent.y)
        );

        glm::vec2 rAW = rotate(bodyA->getPosition().z, contacts[i].rA);
        glm::vec2 rBW = rotate(bodyB->getPosition().z, contacts[i].rB);

        // Precompute the constraint and derivatives at C(x-), since we use a truncated Taylor series for contacts (Sec 4).
        // Note that we discard the second order term, since it is insignificant for contacts
        // GLM matrices are column-major: mat[col][row]
        glm::vec2 basisRow0 = glm::vec2(basis[0][0], basis[1][0]);  // row 0: (normal.x, normal.y)
        glm::vec2 basisRow1 = glm::vec2(basis[0][1], basis[1][1]);  // row 1: (tangent.x, tangent.y)
        setJ(i * 2 + JN, bodyA, glm::vec3(basisRow0.x, basisRow0.y, cross(rAW, normal)));
        setJ(i * 2 + JN, bodyB, glm::vec3(-basisRow0.x, -basisRow0.y, -cross(rBW, normal)));
        setJ(i * 2 + JT, bodyA, glm::vec3(basisRow1.x, basisRow1.y, cross(rAW, tangent)));
        setJ(i * 2 + JT, bodyB, glm::vec3(-basisRow1.x, -basisRow1.y, -cross(rBW, tangent)));

        contacts[i].C0 = basis * (glm::vec2(bodyA->getPosition().x, bodyA->getPosition().y) + rAW - glm::vec2(bodyB->getPosition().x, bodyB->getPosition().y) - rBW) + glm::vec2(COLLISION_MARGIN, 0);
    }

    return numContacts > 0;
}

void Manifold::computeConstraint(float alpha) {
    for (int i = 0; i < numContacts; i++) {
        // Compute the Taylor series approximation of the constraint function C(x) (Sec 4)
        glm::vec3 dpA = bodyA->getPosition() - bodyA->getInitial();
        glm::vec3 dpB = bodyB->getPosition() - bodyB->getInitial();
        
        setC(i * 2 + 0, contacts[i].C0.x * (1 - alpha) + dot(getJ(i * 2 + JN, bodyA), dpA) + dot(getJ(i * 2 + JN, bodyB), dpB));
        setC(i * 2 + 1, contacts[i].C0.y * (1 - alpha) + dot(getJ(i * 2 + JT, bodyA), dpA) + dot(getJ(i * 2 + JT, bodyB), dpB));

        // Update the friction bounds using the latest lambda values
        float frictionBound = glm::abs(getLambda(i * 2 + 0)) * friction;
        setFmax(i * 2 + 1, frictionBound);
        setFmin(i * 2 + 1, -frictionBound);

        // Check if the contact is sticking, so that on the next frame we can use the old contact points for better static friction handling
        contacts[i].stick = glm::abs(getLambda(i * 2 + 1)) < frictionBound && glm::abs(contacts[i].C0.y) < STICK_THRESH;
    }
}

void Manifold::computeDerivatives(Rigid* body) {
    return; // should already be stored, doesn't change after initialization
}

}