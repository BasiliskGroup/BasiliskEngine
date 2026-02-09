#include <basilisk/physics/tables/forceTable.h>
#include <basilisk/physics/forces/force.h>
#include <basilisk/physics/tables/forceTypeTable.h>

namespace bsk::internal {

ForceTable::ForceTable(std::size_t capacity) :
    manifoldTable(new ForceTypeTable<ManifoldData>(capacity, this)),
    jointTable(new ForceTypeTable<JointStruct>(capacity, this)),
    springTable(new ForceTypeTable<SpringStruct>(capacity, this)),
    motorTable(new ForceTypeTable<MotorStruct>(capacity, this))
{
    resize(capacity);
}

ForceTable::~ForceTable() {
    delete manifoldTable; manifoldTable = nullptr;
    delete jointTable; jointTable = nullptr;
    delete springTable; springTable = nullptr;
    delete motorTable; motorTable = nullptr;
}

void ForceTable::markAsDeleted(std::size_t index) {
    toDelete[index] = true;
    forces[index] = nullptr;
}

void ForceTable::resize(std::size_t newCapacity) {
    if (newCapacity <= capacity) return;

    expandTensors(newCapacity,
        forces, toDelete, parameters, derivativesA, derivativesB, specialParameters, forceTypes, rows, positional, indexMap
    );

    capacity = newCapacity;
}

void ForceTable::compact() {
    // do a quick check to see if we need to run more complex compact function
    uint active = numValid(toDelete, size);
    if (active == size) {
        return;
    }

    // Build index map: indexMap[oldIndex] = newIndex (SIZE_MAX for deleted)
    std::size_t dst = 0;
    for (std::size_t src = 0; src < size; ++src) {
        if (!toDelete[src]) {
            indexMap[src] = dst;
            ++dst;
        } else {
            indexMap[src] = SIZE_MAX;
        }
    }

    compactTensors(toDelete, size,
        forces, parameters, derivativesA, derivativesB, specialParameters, forceTypes, rows, positional
    );

    size = active;

    // remap forces
    for (uint i = 0; i < size; i++) {
        toDelete[i] = false;
        forces[i]->setIndex(i);
    }

    // compact type tables
    manifoldTable->compact();
    jointTable->compact();
    springTable->compact();
    motorTable->compact();

    // remap type tables
    manifoldTable->remap();
    jointTable->remap();
    springTable->remap();
    motorTable->remap();
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
    forceTypes[index] = value;

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