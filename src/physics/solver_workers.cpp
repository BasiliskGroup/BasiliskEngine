#include <basilisk/physics/solver.h>

#include <basilisk/physics/rigid.h>
#include <basilisk/physics/forces/force.h>
#include <basilisk/physics/maths.h>

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
        Rigid* body = colorGroups[activeColor][i];
        primalUpdateSingle(primalScratch, body);
    }
}

void Solver::primalUpdateSingle(PrimalScratch& scratch, Rigid* body) {
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
    for (Force* force = body->getForces(); force != nullptr; force = (force->getBodyA() == body) ? force->getNextA() : force->getNextB())
        {
            // Compute constraint and its derivatives
        force->computeConstraint(alpha);
        force->computeDerivatives(body);

        for (int i = 0; i < force->rows(); i++)
        {
            // Use lambda as 0 if it's not a hard constraint
            float stiffness = force->getStiffness(i);
            float lambda = glm::isinf(stiffness) ? force->getLambda(i) : 0.0f;

            // Compute the clamped force magnitude (Sec 3.2)
            float penalty = force->getPenalty(i);
            float f = glm::clamp(penalty * force->getC(i) + lambda, force->getFmin(i), force->getFmax(i));

            // Compute the diagonally lumped geometric stiffness term (Sec 3.5)
            scratch.GoH = force->getH(i, body);
            scratch.GoH = diagonal(length(scratch.GoH[0]), length(scratch.GoH[1]), length(scratch.GoH[2])) * glm::abs(f);

            // Accumulate force (Eq. 13) and hessian (Eq. 17)
            scratch.J = force->getJ(i, body);
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

    std::size_t index = 0;
    for (Force* force = forces; force != nullptr; force = force->getNext()) {
        if (index < range.start || index >= range.end) {
            index++;
            continue;
        }
        dualUpdateSingle(force);
        index++;
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
