#include <basilisk/physics/tables/forceTable.h>
#include <basilisk/physics/forces/force.h>

namespace bsk::internal {

ForceTable::ForceTable(std::size_t capacity) {
    resize(capacity);
}

ForceTable::~ForceTable() {

}

void ForceTable::markAsDeleted(std::size_t index) {
    toDelete[index] = true;
    forces[index] = nullptr;
}

void ForceTable::resize(std::size_t newCapacity) {
    if (newCapacity <= capacity) return;

    expandTensors(newCapacity,
        forces, toDelete, JA, JB, HA, HB, C, fmin, fmax, stiffness, fracture, penalty, lambda, rows
    );

    capacity = newCapacity;
}

void ForceTable::compact() {
    // do a quick check to see if we need to run more complex compact function
    uint active = numValid(toDelete, size);
    if (active == size) {
        return;
    }

    compactTensors(toDelete, size,
        forces, JA, JB, HA, HB, C, fmin, fmax, stiffness, fracture, penalty, lambda, rows
    );

    size = active;

    for (uint i = 0; i < size; i++) {
        toDelete[i] = false;
        forces[i]->setIndex(i);
    }
}

void ForceTable::insert(Force* force) {
    if (this->size >= capacity) {
        resize(capacity * 2);
    }

    // set default arguments
    forces[size] = force;
    toDelete[size] = false;
    rows[size] = 0;

    for (int i = 0; i < MAX_ROWS; i++) {
        JA[size][i] = glm::vec3(0.0f);
        JB[size][i] = glm::vec3(0.0f);
        HA[size][i] = glm::mat3x3(0.0f);
        HB[size][i] = glm::mat3x3(0.0f);
        C[size][i] = 0.0f;
        fmin[size][i] = -INFINITY;
        fmax[size][i] = INFINITY;
        stiffness[size][i] = INFINITY;
        fracture[size][i] = INFINITY;
        penalty[size][i] = 0.0f;
        lambda[size][i] = 0.0f;
    }

    force->setIndex(size);
    size++;
}

}