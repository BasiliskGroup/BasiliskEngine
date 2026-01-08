#include <basilisk/physics/tables/bodyTable.h>
#include <basilisk/physics/rigid.h>
#include <basilisk/nodes/node2d.h>
#include <basilisk/physics/maths.h>
#include <basilisk/physics/collision/bvh.h>


namespace bsk::internal {

BodyTable::BodyTable(std::size_t capacity) : bvh(new BVH()) {
    resize(capacity);   
}

BodyTable::~BodyTable() {
    delete bvh;
    bvh = nullptr;
}

void BodyTable::computeTransforms() {
    // TODO
}

// Initialize and warmstart bodies (ie primal variables)
void BodyTable::warmstartBodies(const float dt, const std::optional<glm::vec3>& gravityOpt) {
    glm::vec3 accel, accelExt, accelWeight, gravity;

    // compute gravity
    if (gravityOpt.has_value() == false) {
        bvh->computeMassProperties();
    }
    
    for (std::size_t i = 0; i < size; i++) {
        // Don't let bodies rotate too fast
        vel[i].z = glm::clamp(vel[i].z, -50.0f, 50.0f);

        // compute gravity
        gravity = gravityOpt.has_value() ? gravityOpt.value() : getGravity(i);

        // Compute inertial position (Eq 2)
        inertial[i] = pos[i] + vel[i] * dt;
        if (mass[i] > 0) {
            inertial[i] += gravity * (dt * dt);
        }
        
        // Adaptive warmstart (See original VBD paper)
        accel = (vel[i] - prevVel[i]) / dt;
        accelExt = accel * glm::sign(gravity);
        accelWeight = glm::clamp(accelExt / glm::abs(gravity), 0.0f, 1.0f);

        // Handle non-finite values per component
        for (int c = 0; c < 3; ++c) {
            if (!std::isfinite(accelWeight[c])) {
                accelWeight[c] = 0.0f;
            }
        }

        // Save initial position (x-) and compute warmstarted position (See original VBD paper)
        initial[i] = pos[i];
        pos[i] += vel[i] * dt + gravity * (accelWeight * dt * dt);
    }
}

// If we are are the final iteration before post stabilization, compute velocities (BDF1)
void BodyTable::updateVelocities(float dt) {
    for (std::size_t i = 0; i < size; i++) {
        // update velocities
        prevVel[i] = vel[i];
        if (mass[i] > 0) {
            vel[i] = (pos[i] - initial[i]) / dt;
        }

        // update node positions
        bodies[i]->getNode()->setPosition(pos[i]);
    }
    
}

void BodyTable::markAsDeleted(std::size_t index) {
    toDelete[index] = true;
    bodies[index] = nullptr;
}

void BodyTable::resize(std::size_t newCapacity) {
    if (newCapacity <= capacity) return;

    expandTensors(newCapacity,
        bodies, toDelete, pos, initial, inertial, vel, prevVel, scale, friction, radius, mass, moment, collider, mat, imat, rmat, updated, color, oldIndex, inverseForceMap, lhs, rhs
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
        collider, mat, imat, rmat, updated, color, oldIndex, lhs, rhs
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

    // insert into bvh
    bvh->insert(body);
}

void BodyTable::writeToNodes() {
    for (uint i = 0; i < size; i++) {
        Node2D* node = bodies[i]->getNode();
        glm::vec3& pos = this->pos[i];
        node->setPosition(pos);
    }
}

glm::vec3 BodyTable::getGravity(Rigid* body) const {
    return getGravity(body->getIndex());
}

glm::vec3 BodyTable::getGravity(std::size_t index) const {
    glm::vec3 gravity = glm::vec3(0.0f);
    glm::vec3 r;
    float lenr;

    // for (std::size_t i = 0; i < size; i++) {
    //     if (i == index) {
    //         continue;
    //     }

    //     // Newtonian gravity formula, acceleration form
    //     r = pos[i] - pos[index];
    //     lenr = glm::length2(r);

    //     if (lenr < EPSILON) {
    //         continue;
    //     }

    //     lenr = glm::sqrt(lenr);
    //     gravity += (mass[i] / (lenr * lenr * lenr)) * r;
    // }

    // return gravity;

    return { bvh->computeGravity(bodies[index]), 0 } ;
}

}