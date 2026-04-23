#ifndef BSK_PHYSICS_FORCES_MANIFOLD_H
#define BSK_PHYSICS_FORCES_MANIFOLD_H

#include <basilisk/physics/forces/force.h>

namespace bsk::internal {

// ------------------------------------------------------------
// Table Struct
// ------------------------------------------------------------
// Contact point information for a single contact
struct Contact {
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
struct alignas(8) GpuContact {
    // don't need feature pair for now
    glm::vec2 rA;
    glm::vec2 rB;
    glm::vec2 normal;
    glm::vec2 C0;
    uint32_t stick;
    uint32_t _pad;
};

struct GpuManifoldData {
    GpuContact contacts[2];
    uint32_t numContacts = 0;
    float friction = 0.5f;

    // padding to 16 bytes
    char _padding[4] = { 0 };
};
static_assert(sizeof(GpuManifoldData) % 16 == 0, "GpuManifoldData must be 16-byte aligned");

// Collision manifold between two rigid bodies, which contains up to two frictional contact points
class Manifold : public Force {
public:
    Manifold(Solver* solver, Rigid* bodyA, Rigid* bodyB);
    Manifold(Solver* solver, Rigid* bodyA, const std::vector<glm::vec2>& worldVerticesB);
    ~Manifold();

    static int rows(ForceTable* forceTable, uint32_t specialIndex);
    int rows() override;
    bool initialize() override;
    static void computeConstraint(ForceTable* forceTable, uint32_t specialIndex, float alpha);
    static void computeDerivatives(ForceTable* forceTable, uint32_t specialIndex, uint32_t bodyIndex, const glm::vec3& jacobianMask);
    static int collide(
        const std::vector<glm::vec2>& worldVerticesA,
        const std::vector<glm::vec2>& worldVerticesB,
        Contact* contacts);
    static int collide(Rigid* bodyA, Rigid* bodyB, Contact* contacts);
    static int collide(Rigid* bodyA, const std::vector<glm::vec2>& worldVerticesB, Contact* contacts);
    
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

    // NOTE this should only be used for static sand ans should be transfered to the GPU after the full migration
private:
    std::vector<glm::vec2> staticWorldVerticesB;
    bool hasStaticWorldShape = false;
};

}

#endif