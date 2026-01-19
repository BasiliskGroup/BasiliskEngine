#include "basilisk/physics/tables/adjacency.h"
#include <basilisk/physics/solver.h>

#include <basilisk/physics/rigid.h>
#include <basilisk/physics/forces/force.h>
#include <basilisk/physics/maths.h>
#include <basilisk/physics/tables/forceTable.h>

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
    if (activeColor < 0 || activeColor >= static_cast<int>(colorGroups.size())) {
        return;
    }
    
    // Get the bodies for this color group - all bodies in the same color can be processed in parallel
    std::size_t colorSize = colorGroups[activeColor].size();
    if (colorSize == 0) {
        return; // Empty color group
    }
    
    PrimalScratch& primalScratch = reinterpret_cast<PrimalScratch&>(scratch.storage);
    WorkRange range = partition(colorSize, threadID, NUM_THREADS);
    
    for (std::size_t i = range.start; i < range.end; i++) {
        primalUpdateSingle(primalScratch, activeColor, i);
    }
}

void Solver::primalUpdateSingle(PrimalScratch& scratch, int activeColor, std::size_t bodyColorIndex) {
    const ColoredData& data = colorGroups[activeColor][bodyColorIndex];
    Rigid* body = data.body; // TODO load mass, moment, and interial in here for entire solving stage

    // Skip static / kinematic bodies
    if (body->getMass() <= 0)
        return;

    // Initialize left and right hand sides of the linear system (Eqs. 5, 6)
    float mass = body->getMass();
    float moment = body->getMoment();
    scratch.lhs = diagonal(mass, mass, moment) / (dt * dt);
    glm::vec3 pos = body->getPosition();
    scratch.rhs = scratch.lhs * (pos - body->getInertial());

    // Iterate over all forces acting on the body
    // Load currentAlpha once per body (it's constant during the stage)
    float alpha = currentAlpha.load(std::memory_order_acquire);
    for (std::size_t i = data.start; i < data.start + data.count; ++i) {
        const ForceEdgeIndices& forceData = forceEdgeIndices[activeColor][i];
        const std::size_t& forceIndex = forceData.force;
        Force* force = forceTable->getForce(forceIndex);

        // Compute constraint and its derivatives
        force->computeConstraint(alpha);
        force->computeDerivatives(body);

        for (int i = 0; i < force->rows(); i++) {
            const ParameterStruct& parameters = forceTable->getParameter(forceIndex, i);
            DerivativeStruct derivatives = (body == force->getBodyA()) ? forceTable->getDerivativeA(forceIndex, i) : forceTable->getDerivativeB(forceIndex, i);

            // Use lambda as 0 if it's not a hard constraint
            float stiffness = parameters.stiffness;
            float lambda = glm::isinf(stiffness) ? parameters.lambda : 0.0f;

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

    // Solve the SPD linear system using LDL and apply the update (Eq. 4)
    pos -= solve(scratch.lhs, scratch.rhs);
    body->setPosition(pos);
}

// ------------------------------------------------------------
// Dual Stage
// ------------------------------------------------------------

void Solver::dualStage(ThreadScratch& scratch, int threadID) {
    WorkRange range = partition(numForces, threadID, NUM_THREADS);

    for (std::size_t i = range.start; i < range.end; i++) {
        Force* force = forceTable->getForce(i);
        dualUpdateSingle(force);
    }
}

void Solver::dualUpdateSingle(Force* force) {
    // Compute constraint
    force->computeConstraint(currentAlpha.load(std::memory_order_acquire));

    for (int i = 0; i < force->rows(); i++)
    {
        // Use lambda as 0 if it's not a hard constraint
        float stiffness = force->getStiffness(i);
        float lambda = glm::isinf(stiffness) ? force->getLambda(i) : 0.0f;

        // Update lambda (Eq 11)
        float penalty = force->getPenalty(i);
        float C = force->getC(i);
        float fmin = force->getFmin(i);
        float fmax = force->getFmax(i);
        float newLambda = glm::clamp(penalty * C + lambda, fmin, fmax);
        force->setLambda(i, newLambda);

        // Disable the force if it has exceeded its fracture threshold
        float fracture = force->getFracture(i);
        if (fabsf(newLambda) >= fracture)
            force->disable();

        // Update the penalty parameter and clamp to material stiffness if we are within the force bounds (Eq. 16)
        if (newLambda > fmin && newLambda < fmax) {
            float newPenalty = glm::min(penalty + beta * glm::abs(C), glm::min(PENALTY_MAX, stiffness));
            force->setPenalty(i, newPenalty);
        }
    }
}

}
