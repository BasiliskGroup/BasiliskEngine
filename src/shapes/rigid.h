#ifndef RIGID_H
#define RIGID_H

class Solver;
class Force;
class Collider;
class BodyTable;

// graphics
class Node2D;

class Rigid {
private:
    Solver* solver;
    Force* forces;
    Rigid* next; // next in solver list
    Rigid* prev; // prev in solver list

    Node2D* node;

    // used to cache relations on system graph
    std::vector<std::pair<uint, ushort>> relations;

    // for Table access
    uint index;

    // stored for copy/move constructor
    float density;

public:
    Rigid(Solver* solver, Node2D* node, vec3 pos, vec2 scale, float density, float friction, vec3 vel, Collider* collider);
    Rigid(Solver* solver, Node2D* node, vec3 pos, vec2 scale, float density, float friction, vec3 vel, uint collider);
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

    vec3 getPos();
    vec2 getScale();
    float getDensity();
    float getFriction();
    vec3 getVel();
    uint getColliderIndex();

    uint getIndex() { return index; }

    void setIndex(uint index) { this->index = index; }
    void setNode(Node2D* node) {this->node = node; }

    // determines if two objects are constrained (no collision needed)
    void precomputeRelations();
    ushort constrainedTo(uint other) const;
    void draw();

private:
    void clear();

};

#endif