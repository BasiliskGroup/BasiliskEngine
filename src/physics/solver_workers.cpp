#include <basilisk/physics/tables/colorTable.h>
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
    assert (colorGroups[activeColor].size() != 0);

    PrimalScratch& primalScratch = reinterpret_cast<PrimalScratch&>(scratch.storage);
    float alpha = currentAlpha.load(std::memory_order_acquire);

    // -------------------------------------------------------
    // Sub-stage 1 (force-parallel): compute rhs and lhs for
    // every force edge in this color group. All edges in the
    // same color are independent so threads can work on any
    // subset without synchronisation.
    // -------------------------------------------------------
    const std::vector<ColorForce>& edges = forceEdgeIndices[activeColor];
    std::size_t edgeCount = edges.size();

    if (edgeCount > 0) {
        WorkRange forceRange = partition(edgeCount, threadID, NUM_THREADS);
        for (std::size_t i = forceRange.start; i < forceRange.end; i++) {
            const ColorForce& edge = edges[i];
            
            switch (edge.type) {
                case ForceType::MANIFOLD:
                    processForce<Manifold>(forceTable->getManifoldTable(), edge.special, edge.bodyIndex, primalScratch, alpha, edge.jacobianMask);
                    break;
                case ForceType::JOINT:
                    processForce<Joint>(forceTable->getJointTable(), edge.special, edge.bodyIndex, primalScratch, alpha, edge.jacobianMask);
                    break;
                case ForceType::SPRING:
                    processForce<Spring>(forceTable->getSpringTable(), edge.special, edge.bodyIndex, primalScratch, alpha, edge.jacobianMask);
                    break;
                case ForceType::MOTOR:
                    processForce<Motor>(forceTable->getMotorTable(), edge.special, edge.bodyIndex, primalScratch, alpha, edge.jacobianMask);
                    break;
                default:
                    break;
            }
        }
    }

    // All threads must finish computing before any body starts accumulating
    dualPassBarrier.arrive_and_wait();

    // -------------------------------------------------------
    // Sub-stage 2 (body-parallel): accumulate the pre-computed
    // per-force rhs/lhs into each body's linear system, then
    // solve and apply the position update.
    // -------------------------------------------------------
    std::size_t colorSize = colorGroups[activeColor].size();
    WorkRange bodyRange = partition(colorSize, threadID, NUM_THREADS);
    for (std::size_t i = bodyRange.start; i < bodyRange.end; i++) {
        primalAccumulateSingle(primalScratch, activeColor, i);
    }
}

template<class TForce, class TForceStruct>
inline void Solver::processForce(ForceTypeTable<TForceStruct>* forceTypeTable, std::size_t specialIndex, uint32_t bodyIndex, PrimalScratch& scratch, float alpha, const glm::vec3& jacobianMask) {
    std::size_t forceIndex = forceTypeTable->getForceIndex(specialIndex);
    TForce::computeConstraint(forceTable, specialIndex, alpha);
    TForce::computeDerivatives(forceTable, specialIndex, bodyIndex, jacobianMask);

    int rows = TForce::rows(forceTable, specialIndex);
    for (int r = 0; r < rows; ++r) {
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
        forceTable->getRhs(forceIndex, r) = scratch.J * f;
        forceTable->getLhs(forceIndex, r) = outer(scratch.J, scratch.J * penalty) + scratch.GoH;
    }
}

void Solver::primalAccumulateSingle(PrimalScratch& scratch, int activeColor, std::size_t bodyColorIndex) {
    const ColorBody& data = colorGroups[activeColor][bodyColorIndex];
    Rigid* body = data.body;

    // Skip static / kinematic bodies
    if (body->getMass() <= 0)
        return;

    // Initialize left and right hand sides of the linear system (Eqs. 5, 6)
    scratch.lhs = diagonal(body->getMass(), body->getMass(), body->getMoment()) / (dt * dt);
    glm::vec3 pos = body->getPosition();
    scratch.rhs = scratch.lhs * (pos - body->getInertial());

    // Accumulate the rhs and lhs contributions already written by sub-stage 1.
    // The force edges for this body are laid out contiguously starting at data.start
    // in the order: manifold, joint, spring, motor.

    // MANIFOLD
    std::size_t start = data.start;
    std::size_t end = data.start + data.manifold;
    for (; start < end; ++start) {
        const ColorForce& forceData = forceEdgeIndices[activeColor][start];
        std::size_t forceIndex = forceTable->getManifoldTable()->getForceIndex(forceData.special);
        int rows = Manifold::rows(forceTable, forceData.special);
        for (int r = 0; r < rows; ++r) {
            scratch.rhs += forceTable->getRhs(forceIndex, r);
            scratch.lhs += forceTable->getLhs(forceIndex, r).asGlm();
        }
    }

    // JOINT
    end = start + data.joint;
    for (; start < end; ++start) {
        const ColorForce& forceData = forceEdgeIndices[activeColor][start];
        std::size_t forceIndex = forceTable->getJointTable()->getForceIndex(forceData.special);
        int rows = Joint::rows(forceTable, forceData.special);
        for (int r = 0; r < rows; ++r) {
            scratch.rhs += forceTable->getRhs(forceIndex, r);
            scratch.lhs += forceTable->getLhs(forceIndex, r).asGlm();
        }
    }

    // SPRING
    end = start + data.spring;
    for (; start < end; ++start) {
        const ColorForce& forceData = forceEdgeIndices[activeColor][start];
        std::size_t forceIndex = forceTable->getSpringTable()->getForceIndex(forceData.special);
        int rows = Spring::rows(forceTable, forceData.special);
        for (int r = 0; r < rows; ++r) {
            scratch.rhs += forceTable->getRhs(forceIndex, r);
            scratch.lhs += forceTable->getLhs(forceIndex, r).asGlm();
        }
    }

    // MOTOR
    end = start + data.motor;
    for (; start < end; ++start) {
        const ColorForce& forceData = forceEdgeIndices[activeColor][start];
        std::size_t forceIndex = forceTable->getMotorTable()->getForceIndex(forceData.special);
        int rows = Motor::rows(forceTable, forceData.special);
        for (int r = 0; r < rows; ++r) {
            scratch.rhs += forceTable->getRhs(forceIndex, r);
            scratch.lhs += forceTable->getLhs(forceIndex, r).asGlm();
        }
    }

    // Solve the SPD linear system using LDL and apply the update (Eq. 4)
    pos -= solve(scratch.lhs, scratch.rhs);
    body->setPosition(pos);
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