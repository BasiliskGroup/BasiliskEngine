#ifndef BSK_BVH_H
#define BSK_BVH_H

#include <basilisk/util/includes.h>
#include <basilisk/physics/collision/primitive.h>

namespace bsk::internal {

class Rigid;

class BVH {
private: 
    Primitive* root;
    uint32_t size;
    std::unordered_map<Rigid*, Primitive*> primitives;
    int rebuildTimer;

public:
    BVH();
    ~BVH();

    // Dynamic BVH operations
    void insert(Rigid* rigid);
    void remove(Rigid* rigid);
    void refit(Rigid* rigid);  // Update a single object's bounds and refit tree
    void refitAll();  // Refit all bounding boxes (call after physics step)
    void rebuild(); // complete rebuild of the BVH
    void update();
    
    // Query operations
    std::vector<Rigid*> query(const glm::vec2& bl, const glm::vec2& tr) const;
    std::vector<Rigid*> query(const glm::vec2& point) const;
    std::vector<Rigid*> query(Rigid* rigid) const;
    
    // Utility
    uint32_t getSize() const { return size; }
    bool isEmpty() const { return root == nullptr; }

    // Gravity operations
    void computeMassProperties();
    glm::vec2 computeGravity(Rigid* rigid);
    
    // Debug/visualization
    std::vector<PrimitiveInfo> getAllPrimitives() const;
    void getSandAABB(Rigid* rigid, glm::vec2& bl, glm::vec2& tr) const;
};

}

#endif