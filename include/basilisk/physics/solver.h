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

#include <basilisk/physics/rigid.h>
#include <basilisk/physics/force.h>

#define PENALTY_MIN 1.0f              // Minimum penalty parameter
#define PENALTY_MAX 1000000000.0f     // Maximum penalty parameter
#define COLLISION_MARGIN 0.0005f      // Margin for collision detection to avoid flickering contacts
#define STICK_THRESH 0.01f            // Position threshold for sticking contacts (ie static friction)

namespace bsk::internal {
    class ColliderTable;
    class BodyTable;
}

namespace bsk::internal {

struct PolytopeFace {
    glm::vec2 normal;
    float distance;
    ushort va;
    ushort vb;

    PolytopeFace() = default;
    PolytopeFace(ushort va, ushort vb, glm::vec2 normal, float distance)
        : normal(normal), distance(distance), va(va), vb(vb) {}
};

using Simplex = std::array<glm::vec2, 3>;

// add 3 since the simplex starts with 3 vertices
using SpSet = std::array<ushort, EPA_ITERATIONS + 3>;
using SpArray = std::array<glm::vec2, EPA_ITERATIONS + 3>;
using Polytope = std::array<PolytopeFace, EPA_ITERATIONS + 3>;

struct CollisionPair {
    // gjk
    Simplex simplex;
    glm::vec2 searchDir;

    // epa // TODO reuse this memory for multiple collision pairs
    SpArray sps;
    SpSet spSet;
    Polytope polytope;

    CollisionPair() = default;
};

// Core solver class which holds all the rigid bodies and forces, and has logic to step the simulation forward in time
class Solver
{
    friend class Rigid;
    friend class Force;
    
private:
    float gravity;      // Gravity
    int iterations;     // Solver iterations
    float dt;           // Timestep

    float alpha;        // Stabilization parameter
    float beta;         // Penalty ramping parameter
    float gamma;        // Warmstarting decay parameter

    bool postStabilize; // Whether to apply post-stabilization to the system

    Rigid* bodies;
    Force* forces;

    ColliderTable* colliderTable;
    BodyTable* bodyTable;

public:
    Solver();
    ~Solver();

    // Linked list management
    void insert(Rigid* body);
    void remove(Rigid* body);
    void insert(Force* force);
    void remove(Force* force);

    void clear();
    void defaultParams();
    void step(float dt);

    // Getters
    ColliderTable* getColliderTable() const { return colliderTable; }
    BodyTable* getBodyTable() const { return bodyTable; }
    Rigid* getBodies() const { return bodies; }
    Force* getForces() const { return forces; }
    float getGravity() const { return gravity; }
    int getIterations() const { return iterations; }
    float getDt() const { return dt; }
    float getAlpha() const { return alpha; }
    float getBeta() const { return beta; }
    float getGamma() const { return gamma; }
    bool getPostStabilize() const { return postStabilize; }
    
    // Setters
    void setGravity(float value) { gravity = value; }
    void setIterations(int value) { iterations = value; }
    void setDt(float value) { dt = value; }
    void setAlpha(float value) { alpha = value; }
    void setBeta(float value) { beta = value; }
    void setGamma(float value) { gamma = value; }
    void setPostStabilize(bool value) { postStabilize = value; }
    void setBodies(Rigid* value) { bodies = value; }
    void setForces(Force* value) { forces = value; }
    
    // Mutable references for friend classes (for linked list operations)
    Rigid*& getBodiesRef() { return bodies; }
    Force*& getForcesRef() { return forces; }
};

}
