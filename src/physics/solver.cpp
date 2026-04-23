#include "basilisk/util/constants.h"
#include <basilisk/physics/solver.h>
#include <basilisk/physics/rigid.h>
#include <basilisk/physics/forces/force.h>
#include <basilisk/physics/forces/manifold.h>
#include <basilisk/util/maths.h>
#include <basilisk/nodes/node2d.h>
#include <basilisk/physics/tables/colliderTable.h>
#include <basilisk/physics/tables/bodyTable.h>
#include <basilisk/physics/tables/forceTable.h>
#include <basilisk/util/time.h>
#include <basilisk/physics/collision/bvh.h>
#include <basilisk/physics/threading/scratch.h>
#include <basilisk/util/fileHandling.h>
#include <basilisk/compute/uniforms.hpp>
#include <basilisk/physics/cellular/cellBuffer.h>
#include <basilisk/physics/cellular/marching.h>


namespace bsk::internal {

std::unique_ptr<ColliderTable> Solver::colliderTable(new ColliderTable(64));

Solver::Solver() : 
    bodies(nullptr), 
    forces(nullptr),
    numRigids(0),
    numForces(0),
    bodyTable(nullptr),
    forceTable(nullptr),
    stageBarrier(NUM_THREADS),
    dualPassBarrier(NUM_THREADS),
    startSignal(0),
    finishSignal(0),
    currentStage(Stage::STAGE_NONE),
    currentAlpha(0.0f),
    currentColor(0),
    running(true),
    workers()
{
    this->bodyTable = new BodyTable(this, 8, velocityShader);

    this->forceTable = new ForceTable(128);
    this->forceTable->setSolver(this);

    this->cellBuffer = new CellBuffer(800, 800, 0.1f);
    this->cellBuffer->initialize("shaders/physics/vertex.glsl", "shaders/physics/fragment.glsl");
    this->cellBuffer->initializeCompute();

    defaultParams();

    workers.reserve(NUM_THREADS);
    for (unsigned int i = 0; i < NUM_THREADS; i++) {
        workers.emplace_back(&Solver::workerLoop, this, i);
    }

    // set up shaders
    rebuildVelocityShader();
}

Solver::~Solver() {
    clear();
}

void Solver::clear() {
    while (forces)
        delete forces;

    while (bodies)
        delete bodies;

    numRigids = 0;
    numForces = 0;

    delete bodyTable;
    bodyTable = nullptr;

    delete forceTable;
    forceTable = nullptr;

    delete cellBuffer;
    cellBuffer = nullptr;

    // Signal workers to exit
    currentStage.store(Stage::STAGE_EXIT, std::memory_order_release);
    startSignal.release(workers.size());

    for (auto& w : workers)
        w.join();
}

void Solver::insert(Rigid* body) {
    if (body == nullptr) {
        return;
    }

    body->setNext(bodies);
    body->setPrev(nullptr);

    if (bodies) {
        bodies->setPrev(body);
    }

    bodies = body;
    numRigids++;
}

void Solver::remove(Rigid* body) {
    if (body == nullptr) {
        return;
    }

    if (body->getPrev()) {
        body->getPrev()->setNext(body->getNext());
    }
    else {
        // This was the head of the list
        bodies = body->getNext();
    }

    if (body->getNext()) {
        body->getNext()->setPrev(body->getPrev());
    }

    // Clear pointers
    body->setNext(nullptr);
    body->setPrev(nullptr);
    numRigids--;
}

void Solver::insert(Force* force) {
    if (force == nullptr) {
        return;
    }

    force->setNext(forces);
    force->setPrev(nullptr);

    if (forces) {
        forces->setPrev(force);
    }

    forces = force;
    numForces++;
}

void Solver::remove(Force* force) {
    if (force == nullptr) {
        return;
    }

    if (force->getPrev()) {
        force->getPrev()->setNext(force->getNext());
    } else {
        // This was the head of the list
        forces = force->getNext();
    }

    if (force->getNext()) {
        force->getNext()->setPrev(force->getPrev());
    }

    // Clear pointers
    force->setNext(nullptr);
    force->setPrev(nullptr);
    numForces--;
}

void Solver::defaultParams() {
    gravity = { 0.0f, -9.81f, 0.0f };
    // gravity = std::nullopt;
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

void Solver::step(float dtIncoming) {    
    this->dt = glm::min(dtIncoming, 1.0f / 20.0f);

    // compact body table
    bodyTable->compact();

    // Perform broadphase collision detection
    bodyTable->getBVH()->update();

    // Use BVH to find potential collisions
    for (Rigid* bodyA = bodies; bodyA != nullptr; bodyA = bodyA->getNext()) {

        // if static, skip
        if (bodyA->getMass() <= 0.0f) continue;

        std::vector<Rigid*> results = bodyTable->getBVH()->query(bodyA);
        for (Rigid* bodyB : results) {
            // Skip pairs in the same non-zero collision group (they ignore each other)
            // checking collision group is cheaper than constrained so it comes first
            int gA = bodyA->getCollisionGroup();
            int gB = bodyB->getCollisionGroup();
            if (gA != 0 && gA == gB) continue;

            // Skip self-collision and already constrained pairs
            if (bodyB == bodyA || bodyA->constrainedTo(bodyB)) continue;
            
            new Manifold(this, bodyA, bodyB);
        }
    }

    // sand collision
    // 1. iterate through all bodies
    // 2. collect sand from AABB
    // 3. turn sand into world vertices
    // 4. create manifold with rigid and all convex comps
    std::vector<uint32_t> sandManifoldIndices;
    const float cellScale = cellBuffer->getCellScale();
    const float halfWidth = static_cast<float>(cellBuffer->getWidth()) * 0.5f;
    const float halfHeight = static_cast<float>(cellBuffer->getHeight()) * 0.5f;

    for (Rigid* body = bodies; body != nullptr; body = body->getNext()) {
        if (body->getMass() <= 0.0f) continue;

        // get pixel-space AABB
        glm::vec2 bl, tr;
        bodyTable->getBVH()->getSandAABB(body, bl, tr);

        int bl_x, bl_y, tr_x, tr_y;
        cellBuffer->worldToPixel(bl, bl_x, bl_y);
        cellBuffer->worldToPixel(tr, tr_x, tr_y);

        if (bl_x > tr_x) std::swap(bl_x, tr_x);
        if (bl_y > tr_y) std::swap(bl_y, tr_y);

        bl_x = glm::clamp(bl_x, 0, cellBuffer->getWidth() - 1);
        tr_x = glm::clamp(tr_x, 0, cellBuffer->getWidth() - 1);
        bl_y = glm::clamp(bl_y, 0, cellBuffer->getHeight() - 1);
        tr_y = glm::clamp(tr_y, 0, cellBuffer->getHeight() - 1);

        if (bl_x > tr_x || bl_y > tr_y) continue;

        // collect sand from AABB
        std::vector<std::vector<int>> sand(tr_x - bl_x + 1, std::vector<int>(tr_y - bl_y + 1, 0));
        for (int x = bl_x; x <= tr_x; x++) {
            for (int y = bl_y; y <= tr_y; y++) {
                Color color = cellBuffer->getActivePixel(x, y);
                if (color.mat_id != 0) {
                    sand[x - bl_x][y - bl_y] = 1;
                }
            }
        }

        MarchingGrid grid(std::move(sand));
        grid.bfs();
        std::vector<MarchComponentGeometry> marchGeom = grid.genMarch();

        // create manifold with rigid and all convex comps
        for (const MarchComponentGeometry& geom : marchGeom) {
            for (const BayazitConvex& convex : geom.convexPieces) {
                if (convex.vertices.size() < 3) continue;

                std::vector<glm::vec2> worldVertices;
                worldVertices.reserve(convex.vertices.size());

                for (const glm::vec2& v : convex.vertices) {
                    // Marching output is local-to-AABB in pixel units.
                    const float px = static_cast<float>(bl_x) + v.x;
                    const float py = static_cast<float>(bl_y) + v.y;
                    worldVertices.emplace_back((px - halfWidth) * cellScale, (py - halfHeight) * cellScale);
                }

                Manifold* manifold = new Manifold(this, body, worldVertices);
                sandManifoldIndices.push_back(manifold->getSpecialIndex());
            }
        }
    }

    // Initialize and warmstart forces
    for (Force* force = forces; force != nullptr;) {
        // Initialization can including caching anything that is constant over the step
        if (!force->initialize()) {
            // Force has returned false meaning it is inactive, so remove it from the solver
            Force* next = force->getNext();
            delete force;
            force = next;
        } else {    
            for (int i = 0; i < force->rows(); i++) {
                if (postStabilize) {
                    // With post stabilization, we can reuse the full lambda from the previous step,
                    // and only need to reduce the penalty parameters
                    float penalty = force->getPenalty(i);
                    force->setPenalty(i, glm::clamp(penalty * gamma, PENALTY_MIN, PENALTY_MAX));
                } else {
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

    bodyTable->warmstartBodies(dt, gravity);

    forceTable->compact();

    // Coloring
    resetColoring();
    dsatur();

    // Main solver loop
    // If using post stabilization, we'll use one extra iteration for the stabilization
    int totalIterations = iterations + (postStabilize ? 1 : 0);
    
    for (int it = 0; it < totalIterations; it++) {
        // If using post stabilization, either remove all or none of the pre-existing constraint error
        float alphaValue = alpha;
        if (postStabilize)
            alphaValue = it < iterations ? 1.0f : 0.0f;
        
        // Store currentAlpha with release ordering to pair with acquire on worker side
        currentAlpha.store(alphaValue, std::memory_order_release);

        // Primal update
        currentStage.store(Stage::STAGE_PRIMAL, std::memory_order_release);

        // iterate through colors - process bodies by color to enable parallel execution
        // Bodies of the same color can be processed in parallel since they have no dependencies
        for (int activeColor = 0; activeColor < colors.tables.size(); activeColor++) {
            // Skip empty color groups (shouldn't happen with proper dsatur, but be safe)
            if (colors.tables[activeColor].bodies.empty()) {
                continue;
            }
            
            currentColor.store(activeColor, std::memory_order_release);
            startSignal.release(NUM_THREADS);
            finishSignal.acquire();
        }

        // Dual update, only for non stabilized iterations in the case of post stabilization
        // If doing more than one post stabilization iteration, we can still do a dual update,
        // but make sure not to persist the penalty or lambda updates done during the stabilization iterations for the next frame.
        if (it < iterations) {
            currentStage.store(Stage::STAGE_DUAL, std::memory_order_release);
            startSignal.release(NUM_THREADS);
            finishSignal.acquire();
        }

        // If we are are the final iteration before post stabilization, compute velocities (BDF1)
        if (it == iterations - 1) {
            bodyTable->updateVelocities(dt);
        }
    }

    // 5. delete all sand manifolds
    ForceTypeTable<ManifoldData>* manifoldTable = forceTable->getManifoldTable();
    for (uint32_t specialIndex : sandManifoldIndices) {
        if (specialIndex >= manifoldTable->getSize()) continue;
        if (manifoldTable->isDeleted(specialIndex)) continue;

        Force* force = manifoldTable->getForce(specialIndex);
        Manifold* manifold = dynamic_cast<Manifold*>(force);
        if (manifold == nullptr) continue;

        // Only clean up transient static-world manifolds created for sand.
        if (manifold->getBodyB() != nullptr) continue;
        delete manifold;
    }
    sandManifoldIndices.clear();
}

// Coloring
void Solver::resetColoring() {
    for (Rigid* body = bodies; body != nullptr; body = body->getNext()) {
        body->resetColoring();
    }

    // Clear priority queue by swapping with empty queue
    ColorQueue empty;
    colorQueue.swap(empty);
    colors.tables.clear();
}

void Solver::dsatur() {
    // Use a set instead of priority_queue for O(log n) updates
    std::set<Rigid*, RigidComparator> colorSet;
    std::vector<std::vector<ColorForce>> tempIndices;

    // add vector in temp indices for each force type
    tempIndices.resize(ForceType::NUM_FORCE_TYPES);
    
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

        // ensure we have enough colors
        colors.tables.resize(color + 1);

        // add body to color group
        colors.tables[color].bodies.emplace_back(body, colors.tables[color].forces.size(), 0, 0, 0, 0);

        // clear temp indices
        for (uint32_t i = 0; i < ForceType::NUM_FORCE_TYPES; i++) {
            tempIndices[i].clear();
        }

        // update uncolored bodies connected to this body
        for (Force* force = body->getForces(); force != nullptr; force = (force->getBodyA() == body) ? force->getNextA() : force->getNextB()) {
            Rigid* other = (force->getBodyA() == body) ? force->getBodyB() : force->getBodyA();

        glm::vec3 jacobianMask = body->getJacobianMask();
            switch (force->getForceType()) {
                case ForceType::JOINT:
                    colors.tables[color].bodies.back().joint++;
                    tempIndices[ForceType::JOINT].emplace_back(force->getSpecialIndex(), body->getIndex(), ForceType::JOINT, jacobianMask);
                    break;
                case ForceType::MANIFOLD:
                    if (body->getResolvesCollisions() == false) {
                        continue;
                    }
                    if (other != nullptr && other->getResolvesCollisions() == false) {
                        continue;
                    }
                    colors.tables[color].bodies.back().manifold++;
                    tempIndices[ForceType::MANIFOLD].emplace_back(force->getSpecialIndex(), body->getIndex(), ForceType::MANIFOLD, jacobianMask);
                    break;
                case ForceType::SPRING:
                    colors.tables[color].bodies.back().spring++;
                    tempIndices[ForceType::SPRING].emplace_back(force->getSpecialIndex(), body->getIndex(), ForceType::SPRING, jacobianMask);
                    break;
                case ForceType::MOTOR:
                    colors.tables[color].bodies.back().motor++;
                    tempIndices[ForceType::MOTOR].emplace_back(force->getSpecialIndex(), body->getIndex(), ForceType::MOTOR, jacobianMask);
                    break;
                default:
                    throw std::runtime_error("Invalid force type");
                    break;
            }

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

        // insert forces in sorted order into edge indices
        for (uint32_t i = 0; i < ForceType::NUM_FORCE_TYPES; i++) {
            colors.tables[color].forces.insert(colors.tables[color].forces.end(), tempIndices[i].begin(), tempIndices[i].end());
        }
    }
}

Rigid* Solver::pick(glm::vec2 at, glm::vec2& local) {
    // Find which body is at the given point
    for (Rigid* body = bodies; body != nullptr; body = body->getNext()) {
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

void Solver::rebuildVelocityShader() {
    delete velocityShader;
    velocityShader = nullptr;

    velocityShader = new ComputeShader(
        readFile(internalPath("shaders/physics/velocity.wgsl").c_str()),
        {
            bodyTable->getPosBuffer()->handle(),
            bodyTable->getInitialBuffer()->handle(),
            bodyTable->getMassBuffer()->handle(),
            bodyTable->getVelBuffer()->handle(),
            bodyTable->getPrevVelBuffer()->handle()
        },
        sizeof(VelocityUniforms)
    );
}

}