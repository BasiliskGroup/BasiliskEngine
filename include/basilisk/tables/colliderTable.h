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
    ColliderTable(std::size_t capacity);
    ~ColliderTable();

    void markAsDeleted(std::size_t index);

    auto& getColliders() { return colliders; }
    auto& getVerts() { return verts; }
    auto& getCom() { return com; }
    auto& getHalfDim() { return halfDim; }
    auto& getArea() { return area; }
    auto& getMoment() { return moment; }

    glm::vec2* getStartPtr(std::size_t index) { return verts[index].data(); }
    std::size_t getLength(std::size_t index) { return verts[index].size(); }

    void resize(std::size_t newCapacity) override;
    void compact() override;
    std::size_t insert(Collider* collider, std::vector<glm::vec2> vertices);
};

}

#endif