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

#include <basilisk/util/includes.h>

#define MAX_ROWS 4                    // Most number of rows an individual constraint can have

namespace bsk::internal {
    class Solver;
    class Rigid;
    class Manifold;
    class Joint;
    class Spring;
    class Motor;
}

namespace bsk::internal {

// Holds all user defined and derived constraint parameters, and provides a common interface for all forces.
class Force
{
    friend class Solver;
    friend class Rigid;
    friend class Joint;
    friend class Spring;
    friend class Motor;
    friend class Manifold;
    
protected:
    Solver* solver;
    Rigid* bodyA;
    Rigid* bodyB;
    Force* nextA;
    Force* nextB;
    Force* next;
    Force* prev;
    Force* prevA;
    Force* prevB;

    glm::vec3 J[MAX_ROWS];
    glm::mat3 H[MAX_ROWS];
    float C[MAX_ROWS];
    float fmin[MAX_ROWS];
    float fmax[MAX_ROWS];
    float stiffness[MAX_ROWS];
    float fracture[MAX_ROWS];
    float penalty[MAX_ROWS];
    float lambda[MAX_ROWS];

public:
    Force(Solver* solver, Rigid* bodyA, Rigid* bodyB);
    virtual ~Force();

    void disable();

    virtual int rows() const = 0;
    virtual bool initialize() = 0;
    virtual void computeConstraint(float alpha) = 0;
    virtual void computeDerivatives(Rigid* body) = 0;
    
    // Getters
    Solver* getSolver() const { return solver; }
    Rigid* getBodyA() const { return bodyA; }
    Rigid* getBodyB() const { return bodyB; }
    Force* getNext() const { return next; }
    Force* getNextA() const { return nextA; }
    Force* getNextB() const { return nextB; }
    Force* getPrev() const { return prev; }
    Force* getPrevA() const { return prevA; }
    Force* getPrevB() const { return prevB; }
    const glm::vec3& getJ(int index) const { return J[index]; }
    const glm::mat3& getH(int index) const { return H[index]; }
    float getC(int index) const { return C[index]; }
    float getFmin(int index) const { return fmin[index]; }
    float getFmax(int index) const { return fmax[index]; }
    float getStiffness(int index) const { return stiffness[index]; }
    float getFracture(int index) const { return fracture[index]; }
    float getPenalty(int index) const { return penalty[index]; }
    float getLambda(int index) const { return lambda[index]; }
    
    // Setters
    void setJ(int index, const glm::vec3& value) { J[index] = value; }
    void setH(int index, const glm::mat3& value) { H[index] = value; }
    void setC(int index, float value) { C[index] = value; }
    void setFmin(int index, float value) { fmin[index] = value; }
    void setFmax(int index, float value) { fmax[index] = value; }
    void setStiffness(int index, float value) { stiffness[index] = value; }
    void setFracture(int index, float value) { fracture[index] = value; }
    void setPenalty(int index, float value) { penalty[index] = value; }
    void setLambda(int index, float value) { lambda[index] = value; }
    
    // Get mutable references for direct access (used by friend classes for performance)
    glm::vec3& getJRef(int index) { return J[index]; }
    glm::mat3& getHRef(int index) { return H[index]; }
    float& getCRef(int index) { return C[index]; }
    float& getStiffnessRef(int index) { return stiffness[index]; }
    float& getPenaltyRef(int index) { return penalty[index]; }
    float& getLambdaRef(int index) { return lambda[index]; }
};

// Revolute joint + angle constraint between two rigid bodies, with optional fracture
class Joint : public Force
{
private:
    glm::vec2 rA, rB;
    glm::vec3 C0;
    float torqueArm;
    float restAngle;

public:
    Joint(Solver* solver, Rigid* bodyA, Rigid* bodyB, glm::vec2 rA, glm::vec2 rB, glm::vec3 stiffness = glm::vec3{ INFINITY, INFINITY, INFINITY },
        float fracture = INFINITY);

    int rows() const override { return 3; }

    bool initialize() override;
    void computeConstraint(float alpha) override;
    void computeDerivatives(Rigid* body) override;
    
    // Getters
    glm::vec2 getRA() const { return rA; }
    glm::vec2 getRB() const { return rB; }
    glm::vec3 getC0() const { return C0; }
    float getTorqueArm() const { return torqueArm; }
    float getRestAngle() const { return restAngle; }
    
    // Setters
    void setRA(const glm::vec2& value) { rA = value; }
    void setRB(const glm::vec2& value) { rB = value; }
    void setC0(const glm::vec3& value) { C0 = value; }
    void setTorqueArm(float value) { torqueArm = value; }
    void setRestAngle(float value) { restAngle = value; }
    
    // Mutable references for friend classes
    glm::vec2& getRARef() { return rA; }
    glm::vec2& getRBRef() { return rB; }
    glm::vec3& getC0Ref() { return C0; }
};

// Standard spring force
class Spring : public Force
{
private:
    glm::vec2 rA, rB;
    float rest;

public:
    Spring(Solver* solver, Rigid* bodyA, Rigid* bodyB, glm::vec2 rA, glm::vec2 rB, float stiffness, float rest = -1);

    int rows() const override { return 1; }

    bool initialize() override { return true; }
    void computeConstraint(float alpha) override;
    void computeDerivatives(Rigid* body) override;
    
    // Getters
    glm::vec2 getRA() const { return rA; }
    glm::vec2 getRB() const { return rB; }
    float getRest() const { return rest; }
    
    // Setters
    void setRA(const glm::vec2& value) { rA = value; }
    void setRB(const glm::vec2& value) { rB = value; }
    void setRest(float value) { rest = value; }
    
    // Mutable references
    glm::vec2& getRARef() { return rA; }
    glm::vec2& getRBRef() { return rB; }
    float& getRestRef() { return rest; }
};

// Force which has no physical effect, but is used to ignore collisions between two bodies
class IgnoreCollision : public Force
{
public:
    IgnoreCollision(Solver* solver, Rigid* bodyA, Rigid* bodyB)
        : Force(solver, bodyA, bodyB) {}

    int rows() const override { return 0; }

    bool initialize() override { return true; }
    void computeConstraint(float alpha) override {}
    void computeDerivatives(Rigid* body) override {}
};

// Motor force which applies a torque to two rigid bodies to achieve a desired angular speed
class Motor : public Force
{
private:
    float speed;

public:
    Motor(Solver* solver, Rigid* bodyA, Rigid* bodyB, float speed, float maxTorque);

    int rows() const override { return 1; }

    bool initialize() override { return true; }
    void computeConstraint(float alpha) override;
    void computeDerivatives(Rigid* body) override;
    
    // Getters
    float getSpeed() const { return speed; }
    
    // Setters
    void setSpeed(float value) { speed = value; }
};

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

        glm::vec3 JAn, JBn, JAt, JBt;
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
