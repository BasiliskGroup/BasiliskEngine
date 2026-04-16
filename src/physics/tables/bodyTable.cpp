#include <basilisk/physics/tables/bodyTable.h>
#include <basilisk/physics/rigid.h>
#include <basilisk/nodes/node2d.h>
#include <basilisk/util/maths.h>
#include <basilisk/physics/collision/bvh.h>
#include <basilisk/util/fileHandling.h>
#include <basilisk/util/print.h>
#include <basilisk/physics/tables/colliderTable.h>
#include <basilisk/compute/uniforms.hpp>

namespace bsk::internal {

BodyTable::BodyTable(Solver* solver, uint32_t capacity, ComputeShader*& velocityShader) : 
    solver(solver),
    bvh(new BVH()),
    posBuffer(new GpuBuffer<bsk::vec3>(capacity)),
    initialBuffer(new GpuBuffer<bsk::vec3>(capacity)),
    inertialBuffer(new GpuBuffer<bsk::vec3>(capacity)),
    velBuffer(new GpuBuffer<bsk::vec3>(capacity)),
    prevVelBuffer(new GpuBuffer<bsk::vec3>(capacity)),
    frictionBuffer(new GpuBuffer<float>(capacity)),
    massBuffer(new GpuBuffer<float>(capacity)),
    momentBuffer(new GpuBuffer<float>(capacity)),
    velStagingBuffer(new StagingBuffer<bsk::vec3>(capacity)),
    prevVelStagingBuffer(new StagingBuffer<bsk::vec3>(capacity)),
    velocityShader(velocityShader)
{
    resize(capacity); 
}

BodyTable::~BodyTable() {
    delete velocityShader; velocityShader = nullptr;

    delete posBuffer;      posBuffer = nullptr;
    delete initialBuffer;  initialBuffer = nullptr;
    delete inertialBuffer; inertialBuffer = nullptr;
    delete velBuffer;      velBuffer = nullptr;
    delete prevVelBuffer;  prevVelBuffer = nullptr;
    delete frictionBuffer; frictionBuffer = nullptr;
    delete massBuffer;     massBuffer = nullptr;
    delete momentBuffer;   momentBuffer = nullptr;
    delete velStagingBuffer;     velStagingBuffer = nullptr;
    delete prevVelStagingBuffer; prevVelStagingBuffer = nullptr;

    delete bvh;            bvh = nullptr;
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
    
    for (uint32_t i = 0; i < size; i++) {
        // Don't let bodies rotate too fast
        vel[i].z = glm::clamp(vel[i].z, -50.0f, 50.0f);

        // compute gravity
        gravity = bodies[i]->getHasGravity() ? (gravityOpt.has_value() ? gravityOpt.value() : getGravity(i)) : glm::vec3(0.0f);

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
    if (dt == 0.0f) return;

    // write all buffers to GPU (TODO remove this once main loop is ported to GPU)
    writeToGpu();

    // set shader uniforms (bodies as u32 to match WGSL layout)
    VelocityUniforms uniforms;
    uniforms.bodies = static_cast<std::uint32_t>(size);
    uniforms.dt = dt;
    velocityShader->setUniform(uniforms);

    GpuEncoder encoder;
    encoder.dispatch(velocityShader->handle(), size, 1, 1);
    encoder.copyToStaging(*velBuffer, *velStagingBuffer);
    encoder.copyToStaging(*prevVelBuffer, *prevVelStagingBuffer);
    encoder.submit();

    velStagingBuffer->mapAsync();
    prevVelStagingBuffer->mapAsync();

    // Read back results from staging buffers after submit.
    velStagingBuffer->collect(vel.data(), vel.size());
    prevVelStagingBuffer->collect(prevVel.data(), prevVel.size());

    for (uint32_t i = 0; i < size; i++) {
        bodies[i]->getNode()->setPosition(pos[i]);
    }
}

void BodyTable::writeToGpu() {
    posBuffer->write(pos);
    initialBuffer->write(initial);
    massBuffer->write(mass);
    velBuffer->write(vel);
    prevVelBuffer->write(prevVel);
}

void BodyTable::markAsDeleted(uint32_t index) {
    toDelete[index] = true;
    bodies[index] = nullptr;
}

void BodyTable::resize(uint32_t newCapacity) {
    if (newCapacity <= capacity) return;

    const bool hadGpuResources = (capacity > 0);

    expandTensors(newCapacity,
    bodies, toDelete, pos, initial, inertial, vel, prevVel, scale, friction, radius, mass, moment, collider, mat, imat, rmat, updated, color, sleeping, jacobianMask, indexMap
    );

    capacity = newCapacity;

    // Recreate GPU buffers and velocity shader only when growing an already-initialized table.
    // Skip on first call from constructor (capacity was 0, velocityShader not yet created).
    if (hadGpuResources) {
        expandGpuBuffers(newCapacity,
            posBuffer,
            initialBuffer,
            inertialBuffer,
            velBuffer,
            prevVelBuffer,
            frictionBuffer,
            massBuffer,
            momentBuffer
        );
        delete velStagingBuffer;
        velStagingBuffer = new StagingBuffer<bsk::vec3>(newCapacity);
        delete prevVelStagingBuffer;
        prevVelStagingBuffer = new StagingBuffer<bsk::vec3>(newCapacity);

        solver->rebuildVelocityShader();
    }
}

void BodyTable::compact() {
    // do a quick check to see if we need to run more complex compact function
    uint32_t active = numValid(toDelete, size);
    if (active == size) {
        return;
    }

    // Build indexMap: oldIndex -> newIndex (-1 for deleted entries)
    uint32_t dst = 0;
    for (uint32_t i = 0; i < size; i++) {
        indexMap[i] = !toDelete[i] ? dst++ : -1;
    }

    // TODO check to see who needs to be compacted and who will just get cleared anyway
    compactTensors(toDelete, size,
bodies, pos, initial, inertial, vel, prevVel, scale, friction, radius, mass, moment, collider, mat, imat, rmat, updated, color, sleeping, jacobianMask
    );

    size = active;

    // remap body indices
    solver->getForceTable()->remapBodyIndices();

    for (uint32_t i = 0; i < size; i++) {
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
    this->sleeping[this->size] = false;
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
    this->jacobianMask[this->size] = glm::vec3(1.0f, 1.0f, 1.0f);

    body->setIndex(this->size);
    this->size++;

    // insert into bvh
    bvh->insert(body);
}

void BodyTable::writeToNodes() {
    for (uint32_t i = 0; i < size; i++) {
        Node2D* node = bodies[i]->getNode();
        glm::vec3& pos = this->pos[i];
        node->setPosition(pos);
    }
}

glm::vec3 BodyTable::getGravity(Rigid* body) const {
    return getGravity(body->getIndex());
}

glm::vec3 BodyTable::getGravity(uint32_t index) const {
    return { bvh->computeGravity(bodies[index]), 0.0f };
}

float BodyTable::getDensity(uint32_t index) {
    Collider* collider = Solver::colliderTable->getCollider(this->collider[index]);
    return mass[index] / (scale[index].x * scale[index].y * collider->getArea());
}

void BodyTable::setDensity(uint32_t index, float value) {
    Collider* collider = Solver::colliderTable->getCollider(this->collider[index]);
    mass[index] = value * (scale[index].x * scale[index].y * collider->getArea());
}

}