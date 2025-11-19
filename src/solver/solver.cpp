#include <basilisk/solver/physics.h>
#include <basilisk/util/debug.h>

namespace bsk::internal {

inline constexpr bool PRINT_TIME = false;
inline constexpr bool DEBUG_MANIFOLD = true;
inline constexpr bool DEBUG_PRIMAL = true;

Solver::Solver() : forces(nullptr), bodies(nullptr) {
    // set default params
    gravity = -10.0f;
    iterations = 10;
    beta = 100000.0f;
    alpha = 0.99f;
    gamma = 0.99f;

    // create Tables
    forceTable = new ForceTable(1024);
    bodyTable = new BodyTable(512);
    colliderTable = new ColliderTable(32);
}

Solver::~Solver() {
    clear();
}

/**
 * @brief Completes a single step for the solver
 * 
 * @param dt time that has passed since last step
 */
void Solver::step(float dt) {
    auto beforeStep = timeNow();
    
    std::cout << "\n========== STEP START ==========\n";
    std::cout << "dt: " << dt << "\n";
    std::cout << "gravity: " << gravity << "\n";
    std::cout << "iterations: " << iterations << "\n";
    std::cout << "alpha: " << alpha << ", beta: " << beta << ", gamma: " << gamma << "\n";
    
    // Count bodies and forces
    int bodyCount = 0;
    for (Rigid* body = bodies; body != nullptr; body = body->getNext()) bodyCount++;
    int forceCount = 0;
    for (Force* force = forces; force != nullptr; force = force->getNext()) forceCount++;
    std::cout << "Bodies: " << bodyCount << ", Forces: " << forceCount << "\n";

    // auto beforeTransform = timeNow();
    // bodyTable->computeTransforms();
    // if (PRINT_TIME) printDurationUS(beforeTransform, timeNow(), "Body Transform:\t\t");

    for (Rigid* body = bodies; body != nullptr; body = body->getNext()) {
        body->computeTransforms();
    }
    sphericalCollision();
    std::cout << "Broad collision pairs: " << collisionPairs.size() << "\n";

    // Delete all manifolds
    std::vector<Force*> toDelete;
    Force* force = forces;
    int numForce = 0;
    while (force != nullptr) {
        if (force->getType() == MANIFOLD) {
            toDelete.push_back(force);
            Force* twin = force->getTwin();
            if (twin != nullptr && force->getNext() == twin) {
                force = twin->getNext();
                continue;
            }
        }
        force = force->getNext();
        numForce++;
    }

    std::cout << "Deleting " << toDelete.size() << " old manifolds\n";
    for (Force* f : toDelete) {
        delete f;
    }

    if (numForce > 2) std::runtime_error("Too many forces");

    narrowCollision();
    print("hey");

    // Count manifolds after narrow collision
    int manifoldCount = 0;
    for (Force* force = forces; force != nullptr; force = force->getNext()) {
        if (force->getType() == MANIFOLD) manifoldCount++;
    }
    std::cout << "Active manifolds after narrow: " << manifoldCount << "\n";

    std::cout << "\n=== WARMSTART MANIFOLDS ===\n";
    for (Force* force = forces; force != nullptr; force = force->getNext()) {
        force->initialize();
        
        std::cout << "Force " << force->getIndex() << " (type=" << force->getType() << "):\n";
        for (uint i = 0; i < force->rows(); i++) {
            std::cout << "  Row " << i << ": lambda=" << force->getLambda()[i] 
                     << " -> " << (force->getLambda()[i] * alpha * gamma)
                     << ", penalty=" << force->getPenalty()[i]
                     << " -> " << glm::clamp(force->getPenalty()[i] * gamma, PENALTY_MIN, PENALTY_MAX) 
                     << ", stiffness=" << force->getStiffness()[i]
                     << "\n";
            
            force->getLambda()[i] = force->getLambda()[i] * alpha * gamma;
            force->getPenalty()[i] = glm::clamp(force->getPenalty()[i] * gamma, PENALTY_MIN, PENALTY_MAX);
            force->getPenalty()[i] = glm::min(force->getPenalty()[i], force->getStiffness()[i]);
        }
    }

    std::cout << "\n=== WARMSTART BODIES ===\n";
    for (Rigid* body = bodies; body != nullptr; body = body->getNext()) {
        std::cout << "Body " << body->getIndex() << ":\n";
        std::cout << "  pos: (" << body->getPos().x << ", " << body->getPos().y << ", " << body->getPos().z << ")\n";
        std::cout << "  vel: (" << body->getVel().x << ", " << body->getVel().y << ", " << body->getVel().z << ")\n";
        std::cout << "  mass: " << body->getMass() << ", moment: " << body->getMoment() << "\n";
        
        body->getInertial() = body->getPos() + body->getVel() * dt;
        if (body->getMass() > 0) 
            body->getInertial() += glm::vec3(0, gravity, 0) * (dt * dt);

        glm::vec3 accel = (body->getVel() - body->getPrevVel()) / dt;
        float accelExt = accel.y * (gravity > 0 ? 1 : -1);
        float accelWeight = glm::clamp(accelExt / abs(gravity), 0.0f, 1.0f);
        if (std::isinf(accelWeight) || gravity == 0) accelWeight = 0.0f;

        body->getInitial() = body->getPos();
        body->getPos() = body->getPos() + body->getVel() * dt + glm::vec3(0, gravity, 0) * (accelWeight * dt * dt);
        
        std::cout << "  inertial: (" << body->getInertial().x << ", " << body->getInertial().y << ", " << body->getInertial().z << ")\n";
        std::cout << "  initial: (" << body->getInitial().x << ", " << body->getInitial().y << ", " << body->getInitial().z << ")\n";
        std::cout << "  warmstart pos: (" << body->getPos().x << ", " << body->getPos().y << ", " << body->getPos().z << ")\n";
        std::cout << "  accelWeight: " << accelWeight << "\n";
    }

    // Main solver loop
    for (uint it = 0; it < iterations; it++) {
        std::cout << "\n--- ITERATION " << it << " ---\n";
        
        // PRIMAL UPDATE
        std::cout << "=== PRIMAL UPDATE ===\n";
        for (Rigid* body = bodies; body != 0; body = body->getNext()) {
            if (body->getMass() <= 0) continue;

            std::cout << "Body " << body->getIndex() << " primal:\n";
            std::cout << "  pos before: (" << body->getPos().x << ", " << body->getPos().y << ", " << body->getPos().z << ")\n";

            glm::mat3x3 M = glm::diagonal3x3(glm::vec3{ body->getMass(), body->getMass(), -body->getMoment() });
            glm::mat3x3 lhs = M / (dt * dt);
            glm::vec3 rhs = M / (dt * dt) * (body->getPos() - body->getInertial());

            std::cout << "  initial rhs: (" << rhs.x << ", " << rhs.y << ", " << rhs.z << ")\n";

            int forceIdx = 0;
            for (Force* force = body->getForces(); force != nullptr; force = force->getNextA()) {
                force->computeConstraint(alpha);
                force->computeDerivatives(body);

                std::cout << "  Force " << force->getIndex() << " (force #" << forceIdx << "):\n";

                for (uint i = 0; i < force->rows(); i++) {
                    float lambda = std::isinf(force->getStiffness()[i]) ? force->getLambda()[i] : 0.0f;
                    float f = glm::clamp(force->getPenalty()[i] * force->getC()[i] + lambda, force->getFmin()[i], force->getFmax()[i]);

                    std::cout << "    Row " << i << ": C=" << force->getC()[i] 
                             << ", penalty=" << force->getPenalty()[i]
                             << ", lambda=" << lambda
                             << ", f=" << f << "\n";
                    std::cout << "    J: (" << force->getJ()[i].x << ", " << force->getJ()[i].y << ", " << force->getJ()[i].z << ")\n";

                    glm::mat3x3 G = glm::diagonal3x3(glm::vec3(
                        glm::length(force->getH()[i][0]), 
                        glm::length(force->getH()[i][1]), 
                        glm::length(force->getH()[i][2])
                    )) * abs(f);

                    rhs += force->getJ()[i] * f;
                    lhs += glm::outerProduct(force->getJ()[i], force->getJ()[i] * force->getPenalty()[i]) + G;
                }
                forceIdx++;
            }

            glm::vec3 x;
            solve(lhs, x, rhs);

            std::cout << "  final rhs: (" << rhs.x << ", " << rhs.y << ", " << rhs.z << ")\n";
            std::cout << "  correction: (" << x.x << ", " << x.y << ", " << x.z << ")\n";

            body->getPos() -= x;
            std::cout << "  pos after: (" << body->getPos().x << ", " << body->getPos().y << ", " << body->getPos().z << ")\n";
        }

        // DUAL UPDATE
        std::cout << "\n=== DUAL UPDATE ===\n";
        for (Force* force = forces; force != nullptr; force = force->getNext()) {
            force->computeConstraint(alpha);

            std::cout << "Force " << force->getIndex() << ":\n";

            for (uint i = 0; i < force->rows(); i++) {
                float lambda = std::isinf(force->getStiffness()[i]) ? force->getLambda()[i] : 0.0f;
                float oldLambda = force->getLambda()[i];
                float oldPenalty = force->getPenalty()[i];

                force->getLambda()[i] = glm::clamp(force->getPenalty()[i] * force->getC()[i] + lambda, force->getFmin()[i], force->getFmax()[i]);

                if (force->getLambda()[i] > force->getFmin()[i] && force->getLambda()[i] < force->getFmax()[i]) {
                    force->getPenalty()[i] = glm::min(force->getPenalty()[i] + beta * abs(force->getC()[i]), glm::min(PENALTY_MAX, force->getStiffness()[i]));
                }

                std::cout << "  Row " << i << ": C=" << force->getC()[i]
                         << ", lambda " << oldLambda << " -> " << force->getLambda()[i]
                         << ", penalty " << oldPenalty << " -> " << force->getPenalty()[i] << "\n";
            }
        }

        // if (forces && it > 3) throw std::runtime_error("done");
    }

    for (Rigid* body = bodies; body != nullptr; body = body->getNext()) {
        body->getPrevVel() = body->getVel();
        if (body->getMass() > 0) {
            body->getVel() = (body->getPos() - body->getInitial()) / dt;
        }
        body->getNode()->setPosition(body->getPos());
    }

    std::cout << "========== STEP END ==========\n\n";
}

void Solver::setGravity(float gravity) {
    this->gravity = gravity;
}

// TODO replace this with BVH
void Solver::sphericalCollision() {
    // clear contact pairs from last frame
    collisionPairs.clear();

    uint bodyCount = 0;
    for (Rigid* body = bodies; body != nullptr; body = body->getNext()) {
        bodyCount++;
    }

    // load body-force relations
    for (Rigid* body = bodies; body != nullptr; body = body->getNext()) {
        body->precomputeRelations();
    }

    glm::vec2 dpos;
    float dy;
    float radsum;
    for (Rigid* bodyA = bodies; bodyA != nullptr; bodyA = bodyA->getNext()) {
        for (Rigid* bodyB = bodyA->getNext(); bodyB != nullptr; bodyB = bodyB->getNext()) {
            uint i = bodyA->getIndex();
            uint j = bodyB->getIndex();

            Force* manifold = nullptr;

            // ignore collision flag
            if (bodyA->constrainedTo(j, manifold) == IGNORE_COLLISION) {
                continue;
            }

            dpos = bodyA->getPos() - bodyB->getPos();
            radsum = bodyA->getRadius() + bodyB->getRadius();
            if (radsum * radsum > glm::length2(dpos)) {
                collisionPairs.emplace_back(bodyA, bodyB, manifold);
            }
        }
    }
}

void Solver::narrowCollision() {
    ColliderRow a, b;
    CollisionPair collisionPair;

    for (const auto& pair : collisionPairs) {
        print("in");

        Rigid* bodyA = pair.bodyA;
        Rigid* bodyB = pair.bodyB;

        initColliderRow(bodyA, a);
        initColliderRow(bodyB, b);

        // determine if objects are colliding
        bool collided = gjk(a, b, collisionPair, 0);

        if (!collided) {
            continue;
        }

        // determine collision normal
        ushort frontIndex = epa(a, b, collisionPair);
        glm::vec2 normal = collisionPair.polytope[frontIndex].normal;

        // TODO determine if we need this check
        if (glm::length2(normal) < 1e-16f) {
            continue;
        }

        normal = glm::normalize(normal);
        if (glm::dot(normal, b.pos - a.pos) > 0) {
            normal *= -1;
        }

        Manifold* aToB = new Manifold(this, bodyA, bodyB, 0); // A -> B
        Manifold* bToA = new Manifold(this, bodyB, bodyA, 1); // B -> A

        collisionPair.manifoldA = aToB;
        collisionPair.manifoldB = bToA;

        // determine object overlap
        sat(a, b, collisionPair);

        print("a");

        aToB->getNormal() = normal;
        bToA->getNormal() = normal;
        aToB->getIsA() = true;
        bToA->getIsA() = false;

        print("b");

        // set twins to allow for proper deletion
        aToB->getTwin() = bToA;
        bToA->getTwin() = aToB;

        print("out");
    }

    print("exit");
}

void Solver::initColliderRow(Rigid* body, ColliderRow& colliderRow) {
    colliderRow.pos = body->getPos();
    colliderRow.scale = body->getScale();
    colliderRow.mat = body->getMat();
    colliderRow.imat = body->getIMat();
    colliderRow.start = colliderTable->getStartPtr(body->getCollider());
    colliderRow.length = colliderTable->getLength(body->getCollider());
}

}