#ifndef BSK_PHYSICS_FORCES_MANIFOLD_H
#define BSK_PHYSICS_FORCES_MANIFOLD_H

#include <basilisk/physics/forces/force.h>

namespace bsk::internal {

// ------------------------------------------------------------
// Table Struct
// ------------------------------------------------------------
// Used to track contact features between frames
union FeaturePair {
    struct Edges {
        char inEdge1;
        char outEdge1;
        char inEdge2;
        char outEdge2;
    } e;
    int value;
};

// Contact point information for a single contact
struct Contact {
    FeaturePair feature;
    glm::vec2 rA;
    glm::vec2 rB;
    glm::vec2 normal;

    bsk::vec3 JAn, JAt, JBn, JBt;
    glm::vec2 C0;
    bool stick;
};

struct ManifoldData {
    Contact contacts[2];
    int numContacts = 0;
    float friction = 0.5f;
};

// Structs for GPU buffers
struct GpuContact {
    // don't need feature pair for now
    glm::vec2 rA;
    glm::vec2 rB;
    glm::vec2 normal;
    glm::vec2 C0;
    bool stick;
};

struct GpuManifoldData {
    GpuContact contacts[2];
    int numContacts = 0;
    float friction = 0.5f;

    // padding to 16 bytes
    char _padding[6] = { 0 };
};

// Collision manifold between two rigid bodies, which contains up to two frictional contact points
class Manifold : public Force {
public:
    Manifold(Solver* solver, Rigid* bodyA, Rigid* bodyB);
    ~Manifold();

    static int rows(ForceTable* forceTable, std::size_t index);
    int rows() override;
    bool initialize() override;
    static void computeConstraint(ForceTable* forceTable, std::size_t index, float alpha);
    static void computeDerivatives(ForceTable* forceTable, std::size_t index, ForceBodyOffset body);

    static int collide(Rigid* bodyA, Rigid* bodyB, Contact* contacts);
    
    // Getters
    const Contact& getContact(int index) const;
    Contact& getContactRef(int index);
    int getNumContacts() const;
    float getFriction() const;
    ManifoldData& getData();
    const ManifoldData& getData() const;
    
    // Setters
    void setNumContacts(int value);
    void setFriction(float value);
    void setData(const ManifoldData& value);
};

}

#endif