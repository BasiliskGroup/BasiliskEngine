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

#include "maths.h"

namespace bsk::internal {
    class Node2D;  // Forward declaration
}

#define MAX_ROWS 4                    // Most number of rows an individual constraint can have
#define PENALTY_MIN 1.0f              // Minimum penalty parameter
#define PENALTY_MAX 1000000000.0f     // Maximum penalty parameter
#define COLLISION_MARGIN 0.0005f      // Margin for collision detection to avoid flickering contacts
#define STICK_THRESH 0.01f            // Position threshold for sticking contacts (ie static friction)

namespace bsk::internal {

struct Rigid;
struct Force;
struct Manifold;
struct Solver;

// Holds all the state for a single rigid body that is needed by AVBD
struct Rigid
{
    Solver* solver;
    Node2D* node;
    Force* forces;
    Rigid* next;
    Rigid* prev;
    glm::vec3 position;
    glm::vec3 initial;
    glm::vec3 inertial;
    glm::vec3 velocity;
    glm::vec3 prevVelocity;
    glm::vec2 size;
    float mass;
    float moment;
    float friction;
    float radius;

    Rigid(Solver* solver, Node2D* node, glm::vec3 position, glm::vec2 size, float density, float friction, glm::vec3 velocity = glm::vec3{ 0, 0, 0 });
    ~Rigid();

    bool constrainedTo(Rigid* other) const;
    
    // Linked list management
    void insert(Force* force);
    void remove(Force* force);
    
    // Setters
    void setPosition(const glm::vec3& pos);
    void setScale(const glm::vec2& scale);
    void setVelocity(const glm::vec3& vel);
    
    // Getters
    glm::vec3 getVelocity() const;
    float getDensity() const;
    float getFriction() const;
    glm::vec3 getVel() const;
    Solver* getSolver() const;
    
    // Node management
    void setNode(void* node);
};

// Holds all user defined and derived constraint parameters, and provides a common interface for all forces.
struct Force
{
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

    Force(Solver* solver, Rigid* bodyA, Rigid* bodyB);
    virtual ~Force();

    void disable();

    virtual int rows() const = 0;
    virtual bool initialize() = 0;
    virtual void computeConstraint(float alpha) = 0;
    virtual void computeDerivatives(Rigid* body) = 0;
};

// Revolute joint + angle constraint between two rigid bodies, with optional fracture
struct Joint : Force
{
    glm::vec2 rA, rB;
    glm::vec3 C0;
    float torqueArm;
    float restAngle;

    Joint(Solver* solver, Rigid* bodyA, Rigid* bodyB, glm::vec2 rA, glm::vec2 rB, glm::vec3 stiffness = glm::vec3{ INFINITY, INFINITY, INFINITY },
        float fracture = INFINITY);

    int rows() const override { return 3; }

    bool initialize() override;
    void computeConstraint(float alpha) override;
    void computeDerivatives(Rigid* body) override;
};

// Standard spring force
struct Spring : Force
{
    glm::vec2 rA, rB;
    float rest;

    Spring(Solver* solver, Rigid* bodyA, Rigid* bodyB, glm::vec2 rA, glm::vec2 rB, float stiffness, float rest = -1);

    int rows() const override { return 1; }

    bool initialize() override { return true; }
    void computeConstraint(float alpha) override;
    void computeDerivatives(Rigid* body) override;
};

// Force which has no physical effect, but is used to ignore collisions between two bodies
struct IgnoreCollision : Force
{
    IgnoreCollision(Solver* solver, Rigid* bodyA, Rigid* bodyB)
        : Force(solver, bodyA, bodyB) {}

    int rows() const override { return 0; }

    bool initialize() override { return true; }
    void computeConstraint(float alpha) override {}
    void computeDerivatives(Rigid* body) override {}
};

// Motor force which applies a torque to two rigid bodies to achieve a desired angular speed
struct Motor : Force
{
    float speed;

    Motor(Solver* solver, Rigid* bodyA, Rigid* bodyB, float speed, float maxTorque);

    int rows() const override { return 1; }

    bool initialize() override { return true; }
    void computeConstraint(float alpha) override;
    void computeDerivatives(Rigid* body) override;
};

// Collision manifold between two rigid bodies, which contains up to two frictional contact points
struct Manifold : Force
{
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

    Contact contacts[2];
    int numContacts;
    float friction;

    Manifold(Solver* solver, Rigid* bodyA, Rigid* bodyB);

    int rows() const override { return numContacts * 2; }

    bool initialize() override;
    void computeConstraint(float alpha) override;
    void computeDerivatives(Rigid* body) override;

    static int collide(Rigid* bodyA, Rigid* bodyB, Contact* contacts);
};

// Core solver class which holds all the rigid bodies and forces, and has logic to step the simulation forward in time
struct Solver
{
    float gravity;      // Gravity
    int iterations;     // Solver iterations
    float dt;           // Timestep

    float alpha;        // Stabilization parameter
    float beta;         // Penalty ramping parameter
    float gamma;        // Warmstarting decay parameter

    bool postStabilize; // Whether to apply post-stabilization to the system

    Rigid* bodies;
    Force* forces;

    Solver();
    ~Solver();

    // Linked list management
    void insert(Rigid* body);
    void remove(Rigid* body);
    void insert(Force* force);
    void remove(Force* force);

    Rigid* pick(glm::vec2 at, glm::vec2& local);
    void clear();
    void defaultParams();
    void step(float dt);
};

}
