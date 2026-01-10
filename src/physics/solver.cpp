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

    // Print number of bodies and forces before coloring
    std::cout << "Bodies: " << numRigids << ", Forces: " << numForces << std::endl;

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
                dualUpdateSingle(force);
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
            if (other == nullptr || other->isColored() || other->isColorUsed(color)) {
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

Rigid* Solver::pick(glm::vec2 at, glm::vec2& local)
{
    // Find which body is at the given point
    for (Rigid* body = bodies; body != nullptr; body = body->getNext())
    {
        glm::mat2 Rt = rotation(-body->getPosition().z);
        glm::vec2 bodyPos = glm::vec2(body->getPosition().x, body->getPosition().y);
        local = Rt * (at - bodyPos);
        glm::vec2 bodySize = body->getSize();
        if (local.x >= -bodySize.x * 0.5f && local.x <= bodySize.x * 0.5f &&
            local.y >= -bodySize.y * 0.5f && local.y <= bodySize.y * 0.5f)
            return body;
    }
    return nullptr;
}

}