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
        forces, toDelete, parameters, derivativesA, derivativesB, specialParameters, forceTypes, rows
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
        forces, parameters, derivativesA, derivativesB, specialParameters, forceTypes, rows
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
        derivativesA[size][i].J = glm::vec3(0.0f);
        derivativesB[size][i].J = glm::vec3(0.0f);
        derivativesA[size][i].H = glm::mat3x3(0.0f);
        derivativesB[size][i].H = glm::mat3x3(0.0f);
        parameters[size][i].C = 0.0f;
        parameters[size][i].fmin = -INFINITY;
        parameters[size][i].fmax = INFINITY;
        parameters[size][i].stiffness = INFINITY;
        parameters[size][i].fracture = INFINITY;
        parameters[size][i].penalty = 0.0f;
        parameters[size][i].lambda = 0.0f;
    }

    force->setIndex(size);
    size++;
}

void ForceTable::setForceType(std::size_t index, ForceType value) {
    SpecialParameters& sp = specialParameters[index];

    switch (forceTypes[index]) {
        case ForceType::MANIFOLD:
            new (&sp.manifold) ManifoldData();
            break;

        case ForceType::JOINT:
            new (&sp.joint) JointStruct();
            break;

        case ForceType::SPRING:
            new (&sp.spring) SpringStruct();
            break;

        case ForceType::MOTOR:
            new (&sp.motor) MotorStruct();
            break;

        default:
            break;
    }
}

}