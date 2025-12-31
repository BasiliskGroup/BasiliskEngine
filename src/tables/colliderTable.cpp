#include <basilisk/tables/colliderTable.h>
#include <basilisk/shapes/collider.h>
#include <basilisk/util/print.h>

namespace bsk::internal {

ColliderTable::ColliderTable(std::size_t capacity) {
    resize(capacity);
}

ColliderTable::~ColliderTable() {
    for (std::size_t i = 0; i < size; i++) {
        if (colliders[i]) {
            delete colliders[i];
            colliders[i] = nullptr;
        }
    }
}

/**
 * @brief Resizes each tensor in the system up to the specified size.
 * 
 * @param newCapacity new capacity of the tensor. If this is below the current size, the function is ignored.
 */
void ColliderTable::resize(std::size_t newCapacity) {
    if (newCapacity <= capacity) return;

    expandTensors(newCapacity,
        colliders, toDelete, verts, com, halfDim, area, moment
    );

    // update capacity
    capacity = newCapacity;
}

// NOTE this function is very expensive but should only be called once per frame
// if needed, find a cheaper solution
void ColliderTable::compact() {
    // do a quick check to see if we need to run more complex compact function
    std::size_t active = numValid(toDelete, size);
    if (active == size) {
        return;
    }

    // Use move semantics for efficient vector-of-vectors compaction
    compactTensors(toDelete, size,
        colliders, verts, com, halfDim, area, moment
    );

    size = active;

    // Update collider indices
    for (std::size_t i = 0; i < size; i++) {
        toDelete[i] = false;
        colliders[i]->setIndex(i);
    }
}

std::size_t ColliderTable::insert(Collider* collider, std::vector<glm::vec2> vertices) {
    if (size == capacity) {
        resize(capacity * 2);
    }

    // insert collider and vertices
    colliders[size] = collider;
    verts[size] = vertices; // NOTE should this be moved?
    toDelete[size] = false;

    // calculate properties
    // find extreme values
    float minX = INFINITY, minY = INFINITY;
    float maxX = -INFINITY, maxY = -INFINITY;
    
    for (std::size_t i = 0; i < verts[size].size(); ++i) {
        glm::vec2& v = verts[size][i];

        if (v.x < minX) minX = v.x;
        if (v.y < minY) minY = v.y;
        if (v.x > maxX) maxX = v.x;
        if (v.y > maxY) maxY = v.y;
    }

    // calculate geometric center
    float gcx = (minX + maxX) / 2;
    float gcy = (minY + maxY) / 2;

    // create half dimensions
    halfDim[size] = { maxX - gcx, maxY - gcy };

    // TODO: Calculate actual COM, area, and moment of inertia
    // For now, use geometric center and placeholder values
    com[size] = { gcx, gcy };
    area[size] = 0.0f;
    moment[size] = 0.0f;

    // increment size
    return size++;
}

void ColliderTable::markAsDeleted(std::size_t index) {
    colliders[index] = nullptr;
    toDelete[index] = true;
}

}