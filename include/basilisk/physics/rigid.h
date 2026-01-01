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

namespace bsk::internal {
    class Node2D;  // Forward declaration
    struct Solver;
    class Collider;
    struct Force;
}

namespace bsk::internal {

// Holds all the state for a single rigid body that is needed by AVBD
struct Rigid
{
    Solver* solver;
    Node2D* node;
    Force* forces;
    Rigid* next;
    Rigid* prev;
    Collider* collider;
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

    Rigid(Solver* solver, Node2D* node, Collider* collider, glm::vec3 position, glm::vec2 size, float density, float friction, glm::vec3 velocity = glm::vec3{ 0, 0, 0 });
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

}

