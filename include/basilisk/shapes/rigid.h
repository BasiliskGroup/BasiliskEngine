#ifndef BSK_RIGID_H
#define BSK_RIGID_H

namespace bsk::internal {

class Solver;
class Force;
class Collider;
class BodyTable;

// graphics
class Node2D;

class Rigid {
public:
    struct Relation {
        uint bodyB;
        Force* force; // shared pointer, do not delete
        ForceType type;
    };

private:
    Solver* solver;
    Force* forces;
    Rigid* next; // next in solver list
    Rigid* prev; // prev in solver list

    Node2D* node;

    // used to cache relations on system graph
    std::vector<Relation> relations;

    // for Table access
    uint index;

    // stored for copy/move constructor
    float density;

    // TEMPORARY
    glm::vec3 pos;
    glm::vec3 initial;
    glm::vec3 inertial;
    glm::vec3 vel;
    glm::vec3 prevVel;
    glm::vec2 scale;
    float friction;
    float mass;
    float moment;
    float radius;
    uint collider;
    glm::mat2x2 mat;
    glm::mat2x2 iMat;
    glm::mat2x2 rMat;
    bool updated;

    // manifold control
    glm::vec3 manifoldMask = glm::vec3(1.0f);
    std::vector<std::string> collisionIgnoreGroups;

public:
    Rigid(Solver* solver, Node2D* node, glm::vec3 pos, glm::vec2 scale, float density, float friction, glm::vec3 vel, Collider* collider, std::vector<std::string> collisionIgnoreGroups = {});
    Rigid(Solver* solver, Node2D* node, glm::vec3 pos, glm::vec2 scale, float density, float friction, glm::vec3 vel, uint collider, std::vector<std::string> collisionIgnoreGroups = {});
    ~Rigid();

    // list management
    void insert(Force* force);
    void remove(Force* force);

    // getters
    Solver*& getSolver() { return solver; }
    Rigid*& getNext()    { return next; }
    Force*& getForces()  { return forces; }
    Rigid*& getPrev()    { return prev; }

    BodyTable* getBodyTable();
    Node2D* getNode() { return node; }
    uint getIndex() { return index; }

    glm::vec3& getPos() { return pos; }
    glm::vec3& getInitial() { return initial; }
    glm::vec3& getInertial() { return inertial; }
    glm::vec3& getVel() { return vel; }
    glm::vec3& getPrevVel() { return prevVel; }
    glm::vec2& getScale() { return scale; }
    float& getFriction() { return friction; }
    float& getMass() { return mass; }
    float& getMoment() { return moment; }
    float& getRadius() { return radius; }
    uint& getCollider() { return collider; }
    glm::mat2x2& getMat() { return mat; }
    glm::mat2x2& getIMat() { return iMat; }
    glm::mat2x2& getRMat() { return rMat; }
    bool& getUpdated() { return updated; }

    float getDensity() { return density; }
    glm::bvec3 getManifoldMask() { return manifoldMask; }
    std::vector<std::string> getCollisionIgnoreGroups() { return collisionIgnoreGroups; }
    
    uint getColliderIndex();

    // setters
    void setPosition(const glm::vec3& pos);
    void setVelocity(const glm::vec3& vel);
    void setScale(const glm::vec2& scale);

    void setIndex(uint index) { this->index = index; }
    void setNode(Node2D* node) { this->node = node; }
    void setManifoldMask(float x, float y, float z) { this->manifoldMask = { x, y, z }; }
    void setCollisionIgnoreGroups(std::vector<std::string> collisionIgnoreGroups) { this->collisionIgnoreGroups = collisionIgnoreGroups; }

    // determines if two objects are constrained (no collision needed)
    void precomputeRelations();
    ForceType constrainedTo(uint other, Force*& force) const;
    void draw();

    // TEMPORARY
    void computeTransforms();

private:
    void clear();

};

}

#endif