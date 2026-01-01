#include <basilisk/physics/tables/colliderTable.h>
#include <basilisk/physics/collision/collider.h>
#include <basilisk/physics/collision/geometry.h>

namespace bsk::internal {

ColliderTable::ColliderTable(std::size_t capacity)
    : VirtualTable()
{
    resize(capacity);
}

ColliderTable::~ColliderTable() {
    // TODO: Don't delete colliders and allow them to be used in multiple scenes/solvers
    for (uint i = 0; i < size; i++) {
        if (colliders[i]) {
            delete colliders[i];
            colliders[i] = nullptr;
        }
    }
}

void ColliderTable::resize(std::size_t newCapacity) {
    // Only expand, never shrink
    if (newCapacity <= capacity) return;
    expandTensors(newCapacity,
        colliders, toDelete, vertices, com, gc, halfDim, area, moment
    );
    capacity = newCapacity;
}

void ColliderTable::compact() {
    // NOTE: This function is very expensive but should only be called once per frame
    // If needed, find a cheaper solution
    // do a quick check to see if we need to run more complex compact function
    std::size_t active = numValid(toDelete, size);
    if (active == size) {
        return;
    }

    // Use move semantics for efficient vector-of-vectors compaction
    compactTensors(toDelete, size,
        colliders, vertices, com, gc, halfDim, area, moment
    );

    size = active;

    // Update collider indices
    for (std::size_t i = 0; i < size; i++) {
        toDelete[i] = false;
        // All colliders after compact should be valid so we don't check for nullptrs
        colliders[i]->setIndex(i);
    }
}

void ColliderTable::insert(Collider* collider, const std::vector<glm::vec2>& vertices) {
    if (size >= capacity) {
        resize(capacity * 2);
    }

    // Insert collider and vertices
    colliders[size] = collider;
    this->vertices[size] = vertices; // NOTE: Should this be moved?
    toDelete[size] = false;

    // Calculate AABB (axis-aligned bounding box)
    auto [min, max] = getAABB(vertices);
    
    // Calculate geometric center (center of AABB)
    gc[size] = (min + max) * 0.5f;
    
    // Calculate half dimensions from AABB
    halfDim[size] = (max - min) * 0.5f;

    // Calculate mass properties (area, moment of inertia, and center of mass)
    auto [areaValue, momentValue] = getMassProperties(vertices, com[size]);
    area[size] = areaValue;
    moment[size] = momentValue;

    // Increment size
    collider->setIndex(size);
    size++;
}

void ColliderTable::markAsDeleted(std::size_t index) {
    // Called when a collider is deleted - marks it for removal during next compact()
    colliders[index] = nullptr;
    toDelete[index] = true;
}

}