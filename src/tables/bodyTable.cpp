#include "tables/bodyTable.h"
#include "scene/sceneRoute.h"
#include "util/print.h"


BodyTable::BodyTable(uint capacity) {
    resize(capacity);
}

/**
 * @brief Computes the scale and rotation matrix and its inverse. Assumes the tensor is compact.
 * 
 */
void BodyTable::computeTransforms() {
    for (uint i = 0; i < size; i++) {
        if (updated[i]) continue;

        float angle = pos[i].z;
        float sx = scale[i].x, sy = scale[i].y;
        float isx = 1 / sx, isy = 1 / sy;

        float c = cos(angle);
        float s = sin(angle);

        rmat[i] = { c, -s, s, c };
        mat[i] = { c * sx, -s * sy, s * sx, c * sy };
        // imat[i] = { c * isx, s * isy, -s * isx, c * isy };

        imat[i] = {  c * isx,  s * isx, -s * isy,  c * isy };

        // rotation-only matrix (clockwise positive)
        // rmat[i] = { c,  s,
        //            -s,  c };

        // // local -> world : rotate (clockwise) then scale
        // mat[i] = { c * sx,  s * sy,
        //           -s * sx,  c * sy };

        // // inverse: world -> local = inverse scale * transpose(rotation)
        // imat[i] = { c * isx,  -s * isx,
        //             s * isy,   c * isy };

        updated[i] = false;
    }
}

void BodyTable::warmstartBodies(const float dt, const float gravity) {
    vec3 gravityVec = { 0, gravity, 0 };
    for (uint i = 0; i < size; i++) {

        // Compute inertial position (Eq 2)
        inertial[i] = pos[i] + vel[i] * dt + gravityVec * (dt * dt) * (float) (mass[i] > 0);

        // Adaptive warmstart (See original VBD paper) TODO
        vec3 accel = (vel[i] - prevVel[i]) / dt;
        float accelExt = accel.y * glm::sign(gravity);
        float accelWeight = glm::clamp(accelExt / abs(gravity), 0.0f, 1.0f);
        accelWeight = std::isfinite(accelWeight) ? accelWeight : 0.0f;

        // Save initial position (x-) and compute warmstarted position (See original VBD paper)
        initial[i] = pos[i];
        pos[i] = initial[i] + vel[i] * dt + gravityVec * (accelWeight * dt * dt);
    }
}

void BodyTable::updateVelocities(float dt) {
    float invdt = 1 / dt;

    for (uint i = 0; i < size; i++) {
        prevVel[i] = vel[i];
        if (mass[i] > 0) {
            vel[i] = invdt * (pos[i] - initial[i]);
        }
    }
}

/**
 * @brief Resizes each tensor in the system up to the specified size. 
 * 
 * @param newCapacity new capacity of the tensor. If this is below the current size, the function is ignored. 
 */
void BodyTable::resize(uint newCapacity) {
    if (newCapacity <= capacity) return;

    expandTensors(newCapacity,
        bodies, toDelete, pos, initial, inertial, vel, prevVel, scale, friction, radius, mass, moment, collider, mat, imat, rmat, updated, color, degree, satur, oldIndex, inverseForceMap, lhs, rhs
    );

    // update capacity
    capacity = newCapacity;
}

// NOTE this function is very expensive but should only be called once per frame
// if needed, find a cheaper solution
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

uint BodyTable::insert(Rigid* body, vec3 pos, vec3 vel, vec2 scale, float friction, float mass, float moment, uint collider, float radius) {
    if (size == capacity) {
        resize(capacity * 2);
    }

    // insert into row
    this->bodies[size] = body;
    this->toDelete[size] = false;
    this->pos[size] = pos;
    this->vel[size] = vel;
    this->scale[size] = scale;
    this->friction[size] = friction;
    this->mass[size] = mass;
    this->moment[size] = moment;
    this->collider[size] = collider;
    this->radius[size] = radius;
    this->updated[size] = false;

    // increment size
    return size++;
}

void BodyTable::markAsDeleted(uint index) {
    bodies[index] = nullptr;
    toDelete[index] = true;
}

void BodyTable::writeToNodes() {
    for (uint i = 0; i < size; i++) {
        Node2D* node = bodies[i]->getNode();
        vec3& pos = this->pos[i];

        // print((long) node);
        // print(pos);
        // print(vel[i]);
        // print("");

        node->setPosition(pos);
    }
}