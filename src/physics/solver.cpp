/*
* Copyright (c) 2025 Chris Giles
*
* Permission to use, copy, modify, distribute and sell this software
* and its documentation for any purpose is hereby granted without fee,
* provided that the above copyright notice appear in all copies.
* Chris Giles makes no representations about the suitability
* of this software for any purpose.
* It is provided "as is" without express or implied warranty.
*/

#include "basilisk/util/constants.h"
#include <basilisk/physics/solver.h>
#include <basilisk/physics/rigid.h>
#include <basilisk/physics/forces/force.h>
#include <basilisk/physics/forces/manifold.h>
#include <basilisk/physics/maths.h>
#include <basilisk/nodes/node2d.h>
#include <basilisk/physics/tables/colliderTable.h>
#include <basilisk/physics/tables/bodyTable.h>
#include <basilisk/util/time.h>
#include <basilisk/physics/collision/bvh.h>
#include <basilisk/physics/threading/scratch.h>
#include <stdexcept>


namespace bsk::internal {

Solver::Solver() : 
    bodies(nullptr), 
    forces(nullptr),
    numRigids(0),
    numForces(0),
    colliderTable(nullptr), 
    bodyTable(nullptr),
    stageBarrier(NUM_THREADS),
    startSignal(0),
    finishSignal(0),
    currentStage(Stage::STAGE_NONE),
    currentAlpha(0.0f),
    currentColor(0),
    running(true),
    workers()
{
    this->colliderTable = new ColliderTable(64);
    this->bodyTable = new BodyTable(128);
    defaultParams();

    workers.reserve(NUM_THREADS);
    for (unsigned int i = 0; i < NUM_THREADS; i++) {
        workers.emplace_back(&Solver::workerLoop, this, i);
    }
}

Solver::~Solver()
{
    clear();
}

void Solver::clear()
{
    while (forces)
        delete forces;

    while (bodies)
        delete bodies;

    numRigids = 0;
    numForces = 0;

    delete colliderTable;
    colliderTable = nullptr;

    // Signal workers to exit
    currentStage.store(Stage::STAGE_EXIT, std::memory_order_release);
    startSignal.release(workers.size());

    for (auto& w : workers)
        w.join();
}

void Solver::insert(Rigid* body)
{
    if (body == nullptr)
    {
        return;
    }

    body->setNext(bodies);
    body->setPrev(nullptr);

    if (bodies)
    {
        bodies->setPrev(body);
    }

    bodies = body;
    numRigids++;
}

void Solver::remove(Rigid* body)
{
    if (body == nullptr)
    {
        return;
    }

    if (body->getPrev())
    {
        body->getPrev()->setNext(body->getNext());
    }
    else
    {
        // This was the head of the list
        bodies = body->getNext();
    }

    if (body->getNext())
    {
        body->getNext()->setPrev(body->getPrev());
    }

    // Clear pointers
    body->setNext(nullptr);
    body->setPrev(nullptr);
    numRigids--;
}

void Solver::insert(Force* force)
{
    if (force == nullptr)
    {
        return;
    }

    force->setNext(forces);
    force->setPrev(nullptr);

    if (forces)
    {
        forces->setPrev(force);
    }

    forces = force;
    numForces++;
}

void Solver::remove(Force* force)
{
    if (force == nullptr)
    {
        return;
    }

    if (force->getPrev())
    {
        force->getPrev()->setNext(force->getNext());
    }
    else
    {
        // This was the head of the list
        forces = force->getNext();
    }

    if (force->getNext())
    {
        force->getNext()->setPrev(force->getPrev());
    }

    // Clear pointers
    force->setNext(nullptr);
    force->setPrev(nullptr);
    numForces--;
}

void Solver::defaultParams()
{
    // gravity = { 0.0f, -9.81f, 0.0f };
    gravity = std::nullopt;
    iterations = 10;

    // Note: in the paper, beta is suggested to be [1, 1000]. Technically, the best choice will
    // depend on the length, mass, and constraint function scales (ie units) of your simulation,
    // along with your strategy for incrementing the penalty parameters.
    // If the value is not in the right range, you may see slower convergance for complex scenes.
    beta = 100000.0f;

    // Alpha controls how much stabilization is applied. Higher values give slower and smoother
    // error correction, and lower values are more responsive and energetic. Tune this depending
    // on your desired constraint error response.
    alpha = 0.99f;
    currentAlpha.store(alpha, std::memory_order_relaxed);

    // Gamma controls how much the penalty and lambda values are decayed each step during warmstarting.
    // This should always be < 1 so that the penalty values can decrease (unless you use a different
    // penalty parameter strategy which does not require decay).
    gamma = 0.99f;

    // Post stabilization applies an extra iteration to fix positional error.
    // This removes the need for the alpha parameter, which can make tuning a little easier.
    postStabilize = true;
}

void Solver::step(float dtIncoming)
{
    auto stepStart = timeNow();
    
    this->dt = glm::min(dtIncoming, 1.0f / 20.0f);

    // Perform broadphase collision detection
    auto broadphaseStart = timeNow();
    bodyTable->getBVH()->update();

    // Use BVH to find potential collisions
    for (Rigid* bodyA = bodies; bodyA != nullptr; bodyA = bodyA->getNext())
    {
        std::vector<Rigid*> results = bodyTable->getBVH()->query(bodyA);
        for (Rigid* bodyB : results)
        {
            // Skip self-collision and already constrained pairs
            if (bodyB == bodyA || bodyA->constrainedTo(bodyB))
                continue;
            
            new Manifold(this, bodyA, bodyB);
        }
    }
    
    auto broadphaseEnd = timeNow();
    printDurationUS(broadphaseStart, broadphaseEnd, "Broadphase: ");

    // Initialize and warmstart forces
    auto warmstartForcesStart = timeNow();
    for (Force* force = forces; force != nullptr;)
    {
        // Initialization can including caching anything that is constant over the step
        if (!force->initialize())
        {
            // Force has returned false meaning it is inactive, so remove it from the solver
            Force* next = force->getNext();
            delete force;
            force = next;
        }
        else
        {
            for (int i = 0; i < force->rows(); i++)
            {
                if (postStabilize)
                {
                    // With post stabilization, we can reuse the full lambda from the previous step,
                    // and only need to reduce the penalty parameters
                    float penalty = force->getPenalty(i);
                    force->setPenalty(i, glm::clamp(penalty * gamma, PENALTY_MIN, PENALTY_MAX));
                }
                else
                {
                    // Warmstart the dual variables and penalty parameters (Eq. 19)
                    // Penalty is safely clamped to a minimum and maximum value
                    float lambda = force->getLambda(i);
                    force->setLambda(i, lambda * alpha * gamma);
                    float penalty = force->getPenalty(i);
                    force->setPenalty(i, glm::clamp(penalty * gamma, PENALTY_MIN, PENALTY_MAX));
                }

                // If it's not a hard constraint, we don't let the penalty exceed the material stiffness
                float penalty = force->getPenalty(i);
                float stiffness = force->getStiffness(i);
                force->setPenalty(i, glm::min(penalty, stiffness));
            }

            force = force->getNext();
        }
    }
    auto warmstartForcesEnd = timeNow();
    printDurationUS(warmstartForcesStart, warmstartForcesEnd, "Warmstart Forces: ");

    auto warmstartBodiesStart = timeNow();
    bodyTable->warmstartBodies(dt, gravity);
    auto warmstartBodiesEnd = timeNow();
    printDurationUS(warmstartBodiesStart, warmstartBodiesEnd, "Warmstart Bodies: ");

    // Coloring
    auto coloringStart = timeNow();
    resetColoring();
    dsatur();
    auto coloringEnd = timeNow();
    printDurationUS(coloringStart, coloringEnd, "Coloring: ");

    // Main solver loop
    // If using post stabilization, we'll use one extra iteration for the stabilization
    int totalIterations = iterations + (postStabilize ? 1 : 0);
    
    auto solverLoopStart = timeNow();
    for (int it = 0; it < totalIterations; it++)
    {
        // If using post stabilization, either remove all or none of the pre-existing constraint error
        float alphaValue = alpha;
        if (postStabilize)
            alphaValue = it < iterations ? 1.0f : 0.0f;
        
        // Store currentAlpha with release ordering to pair with acquire on worker side
        currentAlpha.store(alphaValue, std::memory_order_release);

        // Primal update
        auto primalStart = timeNow();
        currentStage.store(Stage::STAGE_PRIMAL, std::memory_order_release);

        // iterate through colors - process bodies by color to enable parallel execution
        // Bodies of the same color can be processed in parallel since they have no dependencies
        for (int activeColor = 0; activeColor < colorGroups.size(); activeColor++) {
            // Skip empty color groups (shouldn't happen with proper dsatur, but be safe)
            if (colorGroups[activeColor].empty()) {
                continue;
            }
            
            // Store the active color with release ordering - ensures visibility to workers
            currentColor.store(activeColor, std::memory_order_release);
            // Release workers to process this color group
            startSignal.release(NUM_THREADS);
            // Wait for all workers to finish processing this color group
            finishSignal.acquire();
        }

        // old primal update loop
        // for (Rigid* body = bodies; body != nullptr; body = body->getNext())
        // {
        //     primalUpdateSingle(body);
        // }

        auto primalEnd = timeNow();
        printDurationUS(primalStart, primalEnd, "  Primal Update: ");

        // Dual update, only for non stabilized iterations in the case of post stabilization
        // If doing more than one post stabilization iteration, we can still do a dual update,
        // but make sure not to persist the penalty or lambda updates done during the stabilization iterations for the next frame.
        auto dualStart = timeNow();
        if (it < iterations)
        {
            for (Force* force = forces; force != nullptr; force = force->getNext())
            {
                // Compute constraint
                force->computeConstraint(currentAlpha.load(std::memory_order_acquire));

                for (int i = 0; i < force->rows(); i++)
                {
                    // Use lambda as 0 if it's not a hard constraint
                    float stiffness = force->getStiffness(i);
                    float lambda = isinf(stiffness) ? force->getLambda(i) : 0.0f;

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
        auto dualEnd = timeNow();
        if (it < iterations) {
            printDurationUS(dualStart, dualEnd, "  Dual Update: ");
        }

        // If we are are the final iteration before post stabilization, compute velocities (BDF1)
        auto velocityStart = timeNow();
        if (it == iterations - 1)
        {
            bodyTable->updateVelocities(dt);
        }
        auto velocityEnd = timeNow();
        if (it == iterations - 1) {
            printDurationUS(velocityStart, velocityEnd, "  Velocity Update: ");
        }
    }
    auto solverLoopEnd = timeNow();
    printDurationUS(solverLoopStart, solverLoopEnd, "Solver Loop Total: ");
    
    auto stepEnd = timeNow();
    printDurationUS(stepStart, stepEnd, "Step Total: ");
    std::cout << std::endl;
}

// Coloring
void Solver::resetColoring() {
    for (Rigid* body = bodies; body != nullptr; body = body->getNext()) {
        body->resetColoring();
    }

    // Clear priority queue by swapping with empty queue
    ColorQueue empty;
    colorQueue.swap(empty);
    colorGroups.clear();
}

void Solver::dsatur() {
    // Use a set instead of priority_queue for O(log n) updates
    std::set<Rigid*, RigidComparator> colorSet;
    
    // Add all bodies to the set
    for (Rigid* body = bodies; body != nullptr; body = body->getNext()) {
        colorSet.insert(body);
    }

    // Color the bodies
    while (!colorSet.empty()) {
        // Get highest priority element
        auto it = colorSet.end();
        --it;
        Rigid* body = *it;
        colorSet.erase(it);
        
        int color = body->getNextUnusedColor();

        // add color to body
        body->setColor(color);
        body->useColor(color);

        // add body to color group
        colorGroups.resize(color + 1);
        colorGroups[color].push_back(body);

        // update uncolored bodies connected to this body
        for (Force* force = body->getForces(); force != nullptr; force = (force->getBodyA() == body) ? force->getNextA() : force->getNextB()) {
            Rigid* other = (force->getBodyA() == body) ? force->getBodyB() : force->getBodyA();

            // Skip if already colored or has already used this color
            if (other->isColored() || other->isColorUsed(color)) {
                continue;
            }

            // Remove from set, update, re-insert (this triggers re-ordering)
            colorSet.erase(other);
            other->useColor(color);
            other->incrSatur();
            colorSet.insert(other);
        }
    }
    
    // Verify coloring is correct
    for (Rigid* body = bodies; body != nullptr; body = body->getNext()) {
        if (!body->verifyColoring()) {
            throw std::runtime_error("Coloring verification failed: Adjacent rigid bodies have the same color");
        }
    }

    // Print number of colors used
    std::cout << "Number of colors used: " << colorGroups.size() << std::endl;
}

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
            default: 
                break;
        }

        stageBarrier.arrive_and_wait();

        if (threadID == 0) {
            finishSignal.release();
        }
    }
}

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
    
    // Partition the work among threads for this color group
    WorkRange range = partition(colorSize, threadID, NUM_THREADS);

    // Process assigned range of bodies for this color
    for (std::size_t i = range.start; i < range.end; i++) {
        Rigid* body = colorGroups[activeColor][i];
        primalUpdateSingle(body);
    }
}

void Solver::primalUpdateSingle(Rigid* body) {
    // Skip static / kinematic bodies
    if (body->getMass() <= 0)
        return;

    // Initialize left and right hand sides of the linear system (Eqs. 5, 6)
    float mass = body->getMass();
    float moment = body->getMoment();
    glm::mat3 M = diagonal(mass, mass, moment);
    glm::mat3 lhs = M / (dt * dt);
    glm::vec3 pos = body->getPosition();
    glm::vec3 inertial = body->getInertial();
    glm::vec3 rhs = M / (dt * dt) * (pos - inertial);

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
            float lambda = isinf(stiffness) ? force->getLambda(i) : 0.0f;

            // Compute the clamped force magnitude (Sec 3.2)
            float penalty = force->getPenalty(i);
            float C = force->getC(i);
            float fmin = force->getFmin(i);
            float fmax = force->getFmax(i);
            float f = glm::clamp(penalty * C + lambda, fmin, fmax);

            // Compute the diagonally lumped geometric stiffness term (Sec 3.5)
            glm::mat3 H = force->getH(i);
            glm::mat3 G = diagonal(length(H[0]), length(H[1]), length(H[2])) * glm::abs(f);

            // Accumulate force (Eq. 13) and hessian (Eq. 17)
            glm::vec3 J = force->getJ(i);
            rhs += J * f;
            lhs += outer(J, J * penalty) + G;
        }
    }

    // Solve the SPD linear system using LDL and apply the update (Eq. 4)
    pos -= solve(lhs, rhs);
    body->setPosition(pos);
}

}