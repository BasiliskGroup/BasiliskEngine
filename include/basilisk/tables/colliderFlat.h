#ifndef BSK_COLLIDER_FLAT_H
#define BSK_COLLIDER_FLAT_H

#include <basilisk/util/includes.h>
#include <basilisk/tables/virtualTable.h>
#include <basilisk/tables/eraseChunks.h>

namespace bsk::internal {

class ColliderFlat {
private:
    uint vertSize = 0;
    uint vertCapacity;
    uint colliderSize = 0;
    uint colliderCapacity;

    std::unordered_map<uint, Collider*> colliders;
    std::vector<uint> toDelete;

    // store vertex data
    std::vector<glm::vec2> verts;
    std::vector<uint> start;
    std::vector<uint> length;

    // store properties
    std::vector<glm::vec2> com;
    std::vector<glm::vec2> halfDim; // used for OBBs and BVH AABBs
    std::vector<float> area;
    std::vector<float> moment;

public:
    ColliderFlat(uint vertCapacity, uint colliderCapacity);
    ~ColliderFlat() = default;

    std::vector<glm::vec2>& getVerts() { return verts; }
    std::vector<uint>& getStart() { return start; }
    std::vector<uint>& getLength() { return length; }

    uint getStart(uint index) { return start[index]; }
    glm::vec2* getStartPtr(uint index) { return verts.data() + start[index]; }
    uint getLength(uint index) { return length[index]; }

    void compact();
    void resize(uint newCapacity); // TODO make a resize function for colliderSoA
    void refreshPointers();
    uint insert(std::vector<glm::vec2> verts);
    void remove(uint colliderIndex);
};

}

#endif 