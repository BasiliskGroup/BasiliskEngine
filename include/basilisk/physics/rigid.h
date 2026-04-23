#ifndef BSK_PHYSICS_RIGID_H
#define BSK_PHYSICS_RIGID_H

#include <basilisk/util/includes.h>

namespace bsk::internal {

class Node2D;
class Solver;
class Collider;
class Force;
class Joint;
class Spring;
class Motor;
class Manifold;
struct CollisionPair;
struct CollisionData;

// Holds all the state for a single rigid body that is needed by AVBD
class Rigid {
private:
    Solver* solver;
    Node2D* node;
    Force* forces;
    Rigid* next;
    Rigid* prev;
    Collider* collider;

    uint32_t index;

    // TODO move to rigid table
    bool hasGravity = true;

    // Coloring
    int degree;
    int satur;
    std::vector<bool> usedColors;
    bool resolvesCollisions = true;
    int collisionGroup = 0;

public:
    Rigid(Solver* solver, Node2D* node, Collider* collider, glm::vec3 position, glm::vec2 size, float density, float friction, glm::vec3 velocity);
    ~Rigid();

    bool constrainedTo(Rigid* other) const;

    // Coloring
    void resetColoring();
    bool isColored() const;
    bool isColorUsed(int color) const;
    int getNextUnusedColor() const;
    void reserveColors(int count);
    void useColor(int color);
    void incrSatur() { satur++; }
    bool verifyColoring() const;
    
    // Linked list management
    void insert(Force* force);
    void remove(Force* force);
    
    // Setters
    void setPosition(const glm::vec3& pos);
    void setScale(const glm::vec2& scale);
    void setVelocity(const glm::vec3& vel);
    void setInitial(const glm::vec3& initial);
    void setInertial(const glm::vec3& inertial);
    void setPrevVelocity(const glm::vec3& prevVelocity);
    void setMass(float mass);
    void setDensity(float density);
    void setMoment(float moment);
    void setFriction(float friction);
    void setRadius(float radius);
    void setColor(int color);
    void setDegree(int degree);
    void setSatur(int satur);
    void setCollider(Collider* collider);
    void setForces(Force* forces);
    void setNext(Rigid* next);
    void setPrev(Rigid* prev);
    void setNode(Node2D* node);
    void setIndex(uint32_t index);
    void setJacobianMask(const glm::vec3& jacobianMask);
    void setResolvesCollisions(bool resolvesCollisions) { this->resolvesCollisions = resolvesCollisions; }
    void setCollisionGroup(int group) { this->collisionGroup = group; }
    void setHasGravity(bool hasGravity) { this->hasGravity = hasGravity; }

    // Getters
    glm::vec3 getPosition() const;
    glm::vec3 getInitial() const;
    glm::vec3 getInertial() const;
    glm::vec3 getVelocity() const;
    glm::vec3& getVelocityRef();
    glm::vec3 getPrevVelocity() const;
    glm::vec2 getSize() const;
    float getMass() const;
    float getMoment() const;
    float getFriction() const;
    float getRadius() const;
    int getColor() const;
    int getDegree() const;
    int getSatur() const;
    Collider* getCollider() const { return collider; }
    Force* getForces() const { return forces; }
    Rigid* getNext() const { return next; }
    Rigid* getPrev() const { return prev; }
    Node2D* getNode() const { return node; }
    Solver* getSolver() const { return solver; }
    float getDensity() const;
    glm::vec3 getVel() const;
    uint32_t getIndex() const { return index; }
    void getAABB(glm::vec2& bl, glm::vec2& tr) const;
    bool getResolvesCollisions() const { return resolvesCollisions; }
    int getCollisionGroup() const { return collisionGroup; }
    bool getHasGravity() const { return hasGravity; }

    std::vector<CollisionData> getCollisions();
    glm::vec3 getJacobianMask() const;
};

}

#endif