#ifndef BSK_COLLIDER_TABLE_H
#define BSK_COLLIDER_TABLE_H

#include <basilisk/tables/virtualTable.h>
#include <basilisk/util/includes.h>

namespace bsk::internal {

class Collider;

// NOTE we do not need copy or move constructor as we will only have one of these
class ColliderTable : public VirtualTable {
private:
    // columns
    std::vector<Collider*> colliders;
    std::vector<bool> toDelete;
    std::vector<std::vector<glm::vec2>> verts;
    std::vector<glm::vec2> com;
    std::vector<glm::vec2> halfDim;
    std::vector<float> area;
    std::vector<float> moment;

public:
    ColliderTable(uint capacity);
    ~ColliderTable();

    void markAsDeleted(uint index);

    auto& getColliders() { return colliders; }
    auto& getVerts() { return verts; }
    auto& getCom() { return com; }
    auto& getHalfDim() { return halfDim; }
    auto& getArea() { return area; }
    auto& getMoment() { return moment; }

    glm::vec2* getStartPtr(uint index) { return verts[index].data(); }
    uint getLength(uint index) { return verts[index].size(); }

    void resize(uint newCapacity) override;
    void compact() override;
    uint insert(Collider* collider, std::vector<glm::vec2> vertices);
};

}

#endif