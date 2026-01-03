#include <basilisk/physics/tables/bodyTable.h>
#include <basilisk/physics/rigid.h>
#include <basilisk/nodes/node2d.h>

namespace bsk::internal {

BodyTable::BodyTable(std::size_t capacity) {
    resize(capacity);   
}

void BodyTable::computeTransforms() {
    // TODO
}

void BodyTable::warmstartBodies(const float dt, const float gravity) {
    // TODO
}

void BodyTable::updateVelocities(float dt) {
    // TODO
}

void BodyTable::markAsDeleted(std::size_t index) {
    toDelete[index] = true;
    bodies[index] = nullptr;
}

void BodyTable::resize(std::size_t newCapacity) {
    if (newCapacity <= capacity) return;

    expandTensors(newCapacity,
        bodies, toDelete, pos, initial, inertial, vel, prevVel, scale, friction, radius, mass, moment, collider, mat, imat, rmat, updated, color, degree, satur, oldIndex, inverseForceMap, lhs, rhs
    );

    // update capacity
    capacity = newCapacity;
}

void BodyTable::compact() {
    // do a quick check to see if we need to run more complex compact function
    uint active = numValid(toDelete, size);
    if (active == size) {
        return;
    }

    // reset old indices
    for (uint i = 0; i < size; i++) {
        oldIndex[i] = i;
    }

    // TODO check to see who needs to be compacted and who will just get cleared anyway
    compactTensors(toDelete, size,
        bodies, pos, initial, inertial, vel, prevVel,
        scale, friction, radius, mass, moment,
        collider, mat, imat, rmat, updated, color, degree, satur, oldIndex, lhs, rhs
    );

    // invert old indices so that forces can find their new indices
    for (uint i = 0; i < size; i++) {
        inverseForceMap[oldIndex[i]] = i;
    }

    size = active;

    for (uint i = 0; i < size; i++) {
        toDelete[i] = false;
        bodies[i]->setIndex(i);
    }
}

void BodyTable::insert(Rigid* body, glm::vec3 position, glm::vec2 size, float density, float friction, glm::vec3 velocity, Collider* collider) {
    if (this->size >= capacity) {
        resize(capacity * 2);
    }

    // insert into table
    this->bodies[this->size] = body;
    this->toDelete[this->size] = false;
    this->pos[this->size] = position;
    this->vel[this->size] = velocity;
    this->prevVel[this->size] = velocity;
    this->scale[this->size] = size;
    this->friction[this->size] = friction;
    this->mass[this->size] = collider->getMass(size, density);
    this->moment[this->size] = collider->getMoment(size, density);
    this->collider[this->size] = collider->getIndex();
    this->radius[this->size] = collider->getRadius(size);
    this->mat[this->size] = glm::mat2x2(1.0f); // TODO
    this->imat[this->size] = glm::mat2x2(1.0f);
    this->rmat[this->size] = glm::mat2x2(1.0f);
    this->updated[this->size] = false;

    body->setIndex(this->size);
    this->size++;
}

void BodyTable::writeToNodes() {
    for (uint i = 0; i < size; i++) {
        Node2D* node = bodies[i]->getNode();
        glm::vec3& pos = this->pos[i];
        node->setPosition(pos);
    }
}

}