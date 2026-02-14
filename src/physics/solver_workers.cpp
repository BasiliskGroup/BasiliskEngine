#include "basilisk/physics/tables/adjacency.h"
#include <basilisk/physics/solver.h>

#include <basilisk/util/constants.h>
#include <basilisk/physics/rigid.h>
#include <basilisk/physics/forces/force.h>
#include <basilisk/physics/forces/joint.h>
#include <basilisk/physics/forces/manifold.h>
#include <basilisk/physics/forces/motor.h>
#include <basilisk/physics/forces/spring.h>
#include <basilisk/physics/maths.h>
#include <basilisk/physics/tables/forceTable.h>
#include <basilisk/physics/tables/forceTypeTable.h>

namespace bsk::internal {

void Solver::workerLoop(unsigned int threadID) {
    ThreadScratch scratch;

    while (true) {
        startSignal.acquire();
        // The acquire on currentStage.load() ensures we see all writes (including currentAlpha) 
        // that happened before the release store of currentStage
        Stage stage = currentStage.load(std::memory_order_acquire);
        if (stage == Stage::STAGE_EXIT)
            return;

        switch (stage) {
            case Stage::STAGE_PRIMAL:
                primalStage(scratch, threadID, currentColor.load(std::memory_order_acquire)); 
                break;
            case Stage::STAGE_DUAL:
                dualStage(scratch, threadID);
                break;
            default: 
                break;
        }

        // release control back to the main thread
        stageBarrier.arrive_and_wait();

        if (threadID == 0) {
            finishSignal.release();
        }
    }
}

// ------------------------------------------------------------
// Primal Stage
// ------------------------------------------------------------

void Solver::primalStage(ThreadScratch& scratch, int threadID, int activeColor) {
    // Verify activeColor is within bounds (defensive programming)
    assert ((activeColor < 0 || activeColor >= static_cast<int>(colorGroups.size())) == false);
    
    // Get the bodies for this color group - all bodies in the same color can be processed in parallel
    std::size_t colorSize = colorGroups[activeColor].size();
    assert (colorSize != 0);
    
    PrimalScratch& primalScratch = reinterpret_cast<PrimalScratch&>(scratch.storage);
    WorkRange range = partition(colorSize, threadID, NUM_THREADS);
    
    for (std::size_t i = range.start; i < range.end; i++) {
        primalUpdateSingle(primalScratch, activeColor, i);
    }
}

template<class TForce>
inline void Solver::processForce(ForceTable* forceTable, std::size_t forceIndex, ForceBodyOffset body, PrimalScratch& scratch, float alpha) {
    TForce::computeConstraint(forceTable, forceIndex, alpha);
    TForce::computeDerivatives(forceTable, forceIndex, body);

    int rows = TForce::rows(forceTable, forceIndex);
    for (int r = 0; r < TForce::rows(forceTable, forceIndex); ++r) {
        const ParameterStruct& parameters = forceTable->getParameter(forceIndex, r);
        const DerivativeStruct& derivatives = forceTable->getDerivative(forceIndex, r);

        // Use lambda as 0 if it's not a hard constraint
        float stiffness = parameters.stiffness;
        float lambda = glm::isinf(stiffness) ? parameters.lambda : 0.0f; // no branch

        // Compute the clamped force magnitude (Sec 3.2)
        float penalty = parameters.penalty;
        float f = glm::clamp(penalty * parameters.C + lambda, parameters.fmin, parameters.fmax);

        // Compute the diagonally lumped geometric stiffness term (Sec 3.5)
        scratch.GoH = derivatives.H;
        scratch.GoH = diagonal(glm::length(scratch.GoH[0]), glm::length(scratch.GoH[1]), glm::length(scratch.GoH[2])) * glm::abs(f);

        // Accumulate force (Eq. 13) and hessian (Eq. 17)
        scratch.J = derivatives.J;
        scratch.rhs += scratch.J * f;
        scratch.lhs += outer(scratch.J, scratch.J * penalty) + scratch.GoH;
    }
}

void Solver::primalUpdateSingle(PrimalScratch& scratch, int activeColor, std::size_t bodyColorIndex) {
    const ColoredData& data = colorGroups[activeColor][bodyColorIndex];
    Rigid* body = data.body; // TODO load mass, moment, and interial in here for entire solving stage

    // Skip static / kinematic bodies
    if (body->getMass() <= 0)
        return;

    // Initialize left and right hand sides of the linear system (Eqs. 5, 6)
    scratch.lhs = diagonal(data.mass, data.mass, data.moment) / (dt * dt);
    glm::vec3 pos = body->getPosition();
    scratch.rhs = scratch.lhs * (pos - data.inertial);

    // Iterate over all forces acting on the body
    // Load currentAlpha once per body (it's constant during the stage)
    float alpha = currentAlpha.load(std::memory_order_acquire);

    // MANIFOLD
    std::size_t start = data.start;
    std::size_t end = data.start + data.manifold;
    for (; start < end; ++start) {
        const ForceEdgeIndices& forceData = forceEdgeIndices[activeColor][start];
        const std::size_t& forceIndex = forceData.force;
        processForce<Manifold>(forceTable, forceIndex, forceData.offset, scratch, alpha);
    }

    // JOINT
    end = start + data.joint;
    for (; start < end; ++start) {
        const ForceEdgeIndices& forceData = forceEdgeIndices[activeColor][start];
        const std::size_t& forceIndex = forceData.force;
        processForce<Joint>(forceTable, forceIndex, forceData.offset, scratch, alpha);
    }

    // SPRING
    end = start + data.spring;
    for (; start < end; ++start) {
        const ForceEdgeIndices& forceData = forceEdgeIndices[activeColor][start];
        const std::size_t& forceIndex = forceData.force;
        processForce<Spring>(forceTable, forceIndex, forceData.offset, scratch, alpha);
    }

    // MOTOR
    end = start + data.motor;
    for (; start < end; ++start) {
        const ForceEdgeIndices& forceData = forceEdgeIndices[activeColor][start];
        const std::size_t& forceIndex = forceData.force;
        processForce<Motor>(forceTable, forceIndex, forceData.offset, scratch, alpha);
    }

    // Solve the SPD linear system using LDL and apply the update (Eq. 4)
    pos -= solve(scratch.lhs, scratch.rhs);
    body->setPosition(pos);

    // update position in force table
    for (std::size_t i = data.start; i < data.start + data.getCount(); ++i) {
        const ForceEdgeIndices& forceData = forceEdgeIndices[activeColor][i];
        Positional& positional = forceTable->getPositional(forceData.force);
        positional.pos[(std::size_t) forceData.offset] = pos;
    }
}

// ------------------------------------------------------------
// Dual Stage
// ------------------------------------------------------------

void Solver::dualStage(ThreadScratch& scratch, int threadID) {
    // 4-pass dual update: one pass per force type, barrier between passes
    // startSignal semaphore gates stage entry; dualPassBarrier syncs threads between passes

    // Pass 1: Joints
    dualUpdatePass<Joint, JointStruct>(forceTable->getJointTable(), threadID);
    dualPassBarrier.arrive_and_wait();

    // Pass 2: Manifolds
    dualUpdatePass<Manifold, ManifoldData>(forceTable->getManifoldTable(), threadID);
    dualPassBarrier.arrive_and_wait();

    // Pass 3: Springs
    dualUpdatePass<Spring, SpringStruct>(forceTable->getSpringTable(), threadID);
    dualPassBarrier.arrive_and_wait();

    // Pass 4: Motors
    dualUpdatePass<Motor, MotorStruct>(forceTable->getMotorTable(), threadID);
}

}
