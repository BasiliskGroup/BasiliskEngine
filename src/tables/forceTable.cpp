#include <basilisk/tables/forceRoute.h>
#include <basilisk/util/print.h>

namespace bsk::internal {

ForceTable::ForceTable(std::size_t capacity) {
    resize(capacity);

    // create Tables
    manifoldTable = new ManifoldTable(this, capacity);
}

ForceTable::~ForceTable() {
    delete manifoldTable;
}

void ForceTable::markAsDeleted(std::size_t index) { 
    toDelete[index] = true; 
    forces[index] = nullptr;
}

void ForceTable::warmstart(float alpha, float gamma) {
    // print("Force Warmstart");
    for (std::size_t i = 0; i < size; i++) {
        for (std::size_t j = 0; j < ROWS; j++) {
            lambda[i][j] *= alpha * gamma;
            penalty[i][j] = glm::clamp(penalty[i][j] * gamma, PENALTY_MIN, PENALTY_MAX);
            penalty[i][j] = glm::min(penalty[i][j], stiffness[i][j]);
        }
    }
}

void ForceTable::reserveManifolds(std::size_t numPairs, std::size_t& forceIndex, std::size_t& manifoldIndex) {
    manifoldIndex = manifoldTable->reserve(numPairs);
    std::size_t numBodies = 2 * numPairs;
    
    std::size_t neededSpace = pow(2, ceil(log2(size + numBodies)));

    if (neededSpace >= capacity) {
        resize(neededSpace);
    }

    // ensure reserved slots arent deleted and set default values
    for (std::size_t i = size; i < size + numBodies; i++) {
        toDelete[i] = false;

        for (std::size_t j = 0; j < MANIFOLD_ROWS; j++) {
            J[i][j] = glm::vec3(0);
            H[i][j] = glm::mat3x3(0);
            C[i][j]     = 0.0f;
            motor[i][j] = 0.0f;
            stiffness[i][j] =  std::numeric_limits<float>::infinity();
            fmax[i][j]      =  0.0f;
            fmin[i][j]      = -std::numeric_limits<float>::infinity();
            fracture[i][j]  =  std::numeric_limits<float>::infinity();
            penalty[i][j] = 0.0f;
            lambda[i][j]  = 0.0f;
        }
    }

    forceIndex = size;
    size += numBodies;
}

void ForceTable::resize(std::size_t newCapacity) {
    if (newCapacity <= capacity) return;

    expandTensors(newCapacity, 
        forces, toDelete, J, C, motor, stiffness, fracture, fmax, fmin, penalty, lambda, H, type, specialIndex, bodyIndex, isA
    );

    // NOTE we do not have to explicitly set our deletes to false since they outside of size

    // update capacity
    capacity = newCapacity;
}

void ForceTable::compact() {
    // do a quick check to see if we need to run more complex compact function
    std::size_t active = numValid(toDelete, size);
    if (active == size) {
        // nothing to delete
        return;
    }

    // delete marked forces before we lose them in compact
    for(std::size_t i = 0; i < size; i++) {
        if (toDelete[i] == true && forces[i] != nullptr) {
            delete forces[i];
        }
    }

    // todo write new compact function
    compactTensors(toDelete, size, 
        forces, J, C, motor, stiffness, fracture, fmax, fmin, penalty, lambda, H, type, specialIndex, bodyIndex, isA
    );

    size = active;

    // update maps
    for (std::size_t i = 0; i < size; i++) {
        // update force object
        forces[i]->setIndex(i);

        // reset values for toDelete since they were not mutated in the compact
        toDelete[i] = false;
    }

    // compact special tables
    manifoldTable->compact();
}

int ForceTable::insert() {
    // Skipped as requested - you'll handle the special cases
    return 0;
}

}