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
        Force* force;
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

public:
    Rigid(Solver* solver, Node2D* node, glm::vec3 pos, glm::vec2 scale, float density, float friction, glm::vec3 vel, Collider* collider);
    Rigid(Solver* solver, Node2D* node, glm::vec3 pos, glm::vec2 scale, float density, float friction, glm::vec3 vel, uint collider);
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

    glm::vec3& getPos();
    glm::vec3& getInitial();
    glm::vec3& getInertial();
    glm::vec3& getVel();
    glm::vec3& getPrevVel();
    glm::vec2& getScale();
    float& getFriction();
    float& getMass();
    float& getMoment();
    float& getRadius();
    uint& getCollider();
    glm::mat2x2& getMat();
    glm::mat2x2& getIMat();
    glm::mat2x2& getRMat();
    bool getUpdated();

    float getDensity() { return density; }
    
    uint getColliderIndex();

    // setters
    void setPosition(const glm::vec3& pos);
    void setVelocity(const glm::vec3& vel);

    void setIndex(uint index) { this->index = index; }
    void setNode(Node2D* node) {this->node = node; }

    // determines if two objects are constrained (no collision needed)
    void precomputeRelations();
    ForceType constrainedTo(uint other, Force*& force) const;
    void draw();

private:
    void clear();

};

}

#endif