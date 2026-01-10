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

// Collision manifold between two rigid bodies, which contains up to two frictional contact points
class Manifold : public Force
{
public:
    // Used to track contact features between frames
    union FeaturePair
    {
        struct Edges
        {
            char inEdge1;
            char outEdge1;
            char inEdge2;
            char outEdge2;
        } e;
        int value;
    };

    // Contact point information for a single contact
    struct Contact
    {
        FeaturePair feature;
        glm::vec2 rA;
        glm::vec2 rB;
        glm::vec2 normal;
        glm::vec2 C0;
        bool stick;
    };

private:
    Contact contacts[2];
    int numContacts;
    float friction;

public:
    Manifold(Solver* solver, Rigid* bodyA, Rigid* bodyB);

    int rows() const override { return numContacts * 2; }

    bool initialize() override;
    void computeConstraint(float alpha) override;
    void computeDerivatives(Rigid* body) override;

    static int collide(Rigid* bodyA, Rigid* bodyB, Contact* contacts);
    
    // Getters
    const Contact& getContact(int index) const { return contacts[index]; }
    Contact& getContactRef(int index) { return contacts[index]; }
    int getNumContacts() const { return numContacts; }
    float getFriction() const { return friction; }
    
    // Setters
    void setNumContacts(int value) { numContacts = value; }
    void setFriction(float value) { friction = value; }
};

}

