#include <basilisk/physics/tables/forceTable.h>
#include <basilisk/physics/forces/force.h>
#include <basilisk/physics/tables/forceTypeTable.h>
#include <basilisk/physics/rigid.h>
#include <basilisk/physics/solver.h>
#include <basilisk/physics/tables/bodyTable.h>

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
        forces, toDelete, parameters, derivatives, forceTypes, rows, bodies, indexMap
    );

    capacity = newCapacity;
}

void ForceTable::compact() {
    // do a quick check to see if we need to run more complex compact function
    uint active = numValid(toDelete, size);
    if (active == size) {
        return;
    }

    // Build index map: indexMap[oldIndex] = newIndex (-1 for deleted)
    std::size_t dst = 0;
    for (std::size_t src = 0; src < size; ++src) {
        indexMap[src] = !toDelete[src] ? dst++ : -1;
    }

    compactTensors(toDelete, size,
        forces, parameters, derivatives, forceTypes, rows, bodies
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

    bodies[size].a = force->getBodyA() ? force->getBodyA()->getIndex() : -1;
    bodies[size].b = force->getBodyB() ? force->getBodyB()->getIndex() : -1;

    for (int i = 0; i < MAX_ROWS; i++) {
        derivatives[size][i].J = glm::vec3(0.0f);
        derivatives[size][i].H = glm::mat3x3(0.0f);
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
    forceTypes[index] = value;
}

void ForceTable::printIndices() const {
    std::cout << "Manifold indices: " << std::endl;
    manifoldTable->printIndices();
    std::cout << "Joint indices: " << std::endl;
    jointTable->printIndices();
    std::cout << "Spring indices: " << std::endl;
    springTable->printIndices();
    std::cout << "Motor indices: " << std::endl;
    motorTable->printIndices();
}

glm::vec3 ForceTable::getPosA(std::size_t index) {
    return bodies[index].a != -1 ? solver->getBodyTable()->getPos(bodies[index].a) : glm::vec3(0.0f);
}

glm::vec3 ForceTable::getPosB(std::size_t index) {
    return bodies[index].b != -1 ? solver->getBodyTable()->getPos(bodies[index].b) : glm::vec3(0.0f);
}

glm::vec3 ForceTable::getInitialA(std::size_t index) {
    return bodies[index].a != -1 ? solver->getBodyTable()->getInitial(bodies[index].a) : glm::vec3(0.0f);
}

glm::vec3 ForceTable::getInitialB(std::size_t index) {
    return bodies[index].b != -1 ? solver->getBodyTable()->getInitial(bodies[index].b) : glm::vec3(0.0f);
}

void ForceTable::setPosA(std::size_t index, const glm::vec3& value) {
    if (bodies[index].a != -1) {
        solver->getBodyTable()->setPos(bodies[index].a, value);
    }
}

void ForceTable::setPosB(std::size_t index, const glm::vec3& value) {
    if (bodies[index].b != -1) {
        solver->getBodyTable()->setPos(bodies[index].b, value);
    }
}

void ForceTable::setInitialA(std::size_t index, const glm::vec3& value) {
    if (bodies[index].a != -1) {
        solver->getBodyTable()->setInitial(bodies[index].a, value);
    }
}

void ForceTable::setInitialB(std::size_t index, const glm::vec3& value) {
    if (bodies[index].b != -1) {
        solver->getBodyTable()->setInitial(bodies[index].b, value);
    }
}

void ForceTable::remapBodyIndices() {
    for (uint i = 0; i < size; i++) {
        if (bodies[i].a != -1) {
            bodies[i].a = solver->getBodyTable()->getMappedIndex(bodies[i].a);
        }
        if (bodies[i].b != -1) {
            bodies[i].b = solver->getBodyTable()->getMappedIndex(bodies[i].b);
        }
    }
}

}