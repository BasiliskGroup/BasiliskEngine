#include <basilisk/solver/physics.h>
#include <basilisk/util/debug.h>

namespace bsk::internal {

inline constexpr bool PRINT_TIME = false;
inline constexpr bool DEBUG_MANIFOLD = true;
inline constexpr bool DEBUG_PRIMAL = true;

Solver::Solver() : forces(nullptr), bodies(nullptr) {
    // set default params
    gravity = -9.8f;
    iterations = 8;
    beta = 1.0e9f;
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
    
    // compute rigid transforms for frame
    for (Rigid* body = bodies; body != nullptr; body = body->getNext()) {
        body->computeTransforms();
    }
    sphericalCollision();
    narrowCollision();

    for (Force* force = forces; force != nullptr; force = force->getNext()) {        
        for (uint i = 0; i < force->rows(); i++) {
            force->getLambda()[i] = force->getLambda()[i] * alpha * gamma;
            force->getPenalty()[i] = glm::clamp(force->getPenalty()[i] * gamma, PENALTY_MIN, PENALTY_MAX);
            force->getPenalty()[i] = glm::min(force->getPenalty()[i], force->getStiffness()[i]);
        }
    }

    for (Rigid* body = bodies; body != nullptr; body = body->getNext()) {
        body->getInertial() = body->getPos() + body->getVel() * dt;
        if (body->getMass() > 0) 
            body->getInertial() += glm::vec3(0, gravity, 0) * (dt * dt);

        glm::vec3 accel = (body->getVel() - body->getPrevVel()) / dt;
        float accelExt = accel.y * (gravity > 0 ? 1 : -1);
        float accelWeight = glm::clamp(accelExt / abs(gravity), 0.0f, 1.0f);
        if (std::isinf(accelWeight) || gravity == 0) accelWeight = 0.0f;

        body->getInitial() = body->getPos();
        body->getPos() = body->getPos() + body->getVel() * dt + glm::vec3(0, gravity, 0) * (accelWeight * dt * dt);
    }

    // Main solver loop
    for (uint it = 0; it < iterations; it++) {
        // PRIMAL UPDATE
        for (Rigid* body = bodies; body != 0; body = body->getNext()) {
            if (body->getMass() <= 0) continue;

            glm::mat3x3 M = glm::diagonal3x3(glm::vec3{ body->getMass(), body->getMass(), body->getMoment() });
            glm::mat3x3 lhs = M / (dt * dt);
            glm::vec3 rhs = M / (dt * dt) * (body->getPos() - body->getInertial());

            int forceIdx = 0;
            for (Force* force = body->getForces(); force != 0; force = (force->getBodyA() == body) ? force->getNextA() : force->getNextB()) {
                force->computeConstraint(alpha);
                force->computeDerivatives(body);

                for (uint i = 0; i < force->rows(); i++) {
                    float lambda = std::isinf(force->getStiffness()[i]) ? force->getLambda()[i] : 0.0f;
                    float f = glm::clamp(force->getPenalty()[i] * force->getC()[i] + lambda, force->getFmin()[i], force->getFmax()[i]);

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
            body->getPos() -= x;
        }

        // DUAL UPDATE
        for (Force* force = forces; force != nullptr; force = force->getNext()) {
            force->computeConstraint(alpha);

            for (uint i = 0; i < force->rows(); i++) {
                float lambda = std::isinf(force->getStiffness()[i]) ? force->getLambda()[i] : 0.0f;
                float oldLambda = force->getLambda()[i];
                float oldPenalty = force->getPenalty()[i];

                force->getLambda()[i] = glm::clamp(force->getPenalty()[i] * force->getC()[i] + lambda, force->getFmin()[i], force->getFmax()[i]);

                if (force->getLambda()[i] > force->getFmin()[i] && force->getLambda()[i] < force->getFmax()[i]) {
                    force->getPenalty()[i] = glm::min(force->getPenalty()[i] + beta * abs(force->getC()[i]), glm::min(PENALTY_MAX, force->getStiffness()[i]));
                }
            }
        }
    }

    for (Rigid* body = bodies; body != nullptr; body = body->getNext()) {
        body->getPrevVel() = body->getVel();
        if (body->getMass() > 0) {
            body->getVel() = (body->getPos() - body->getInitial()) / dt;
        }
        body->getNode()->setPosition(body->getPos());
    }
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
            if (bodyA->getMass() < 0 && bodyB->getMass() < 0) continue;
            if (glm::all(glm::epsilonEqual(bodyA->getPos(), bodyB->getPos(), 1e-10f))) continue;

            // check collision ignore groups // TODO switch to using ignore collision flags
            bool match = false;
            for (const std::string& groupA : bodyA->getCollisionIgnoreGroups()) {
                for (const std::string& groupB : bodyB->getCollisionIgnoreGroups()) {
                    if (groupA != groupB) continue;
                    match = true;
                    break;
                }

                if (match) {
                    break;
                }
                
            }

            if (match) {
                continue;
            }

            // check if there is a constraint between the two bodies
            Force* manifold = bodyA->getConstraint(bodyB);
            if (manifold != nullptr && manifold->getType() == MANIFOLD) {
                collisionPairs.emplace_back(bodyA, bodyB, (Manifold*) manifold);
                continue;
            }

            // if there is no constraint, check if the two bodies could be colliding
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
        Rigid* bodyA = pair.bodyA;
        Rigid* bodyB = pair.bodyB;

        // Use existing manifold if available, otherwise start with nullptr
        collisionPair.manifold = pair.manifold ? static_cast<Manifold*>(pair.manifold) : nullptr;

        initColliderRow(bodyA, a);
        initColliderRow(bodyB, b);

        // determine if objects are colliding
        bool collided = gjk(a, b, collisionPair, 0);

        if (!collided) {
            // Delete manifold if collision has ended (whether existing or newly created)
            if (collisionPair.manifold != nullptr) {
                delete collisionPair.manifold;
                collisionPair.manifold = nullptr;
            }
            continue;
        }

        // determine collision normal
        ushort frontIndex = epa(a, b, collisionPair);
        glm::vec2 normal = collisionPair.polytope[frontIndex].normal;

        // TODO determine if we need this check
        if (glm::length2(normal) < 1e-10f) {
            // Delete manifold if collision validation fails (whether existing or newly created)
            if (collisionPair.manifold != nullptr) {
                delete collisionPair.manifold;
                collisionPair.manifold = nullptr;
            }
            continue;
        }

        normal = glm::normalize(normal);
        if (glm::dot(normal, b.pos - a.pos) > 0) {
            normal *= -1;
        }

        // set collision normal and create manifold if needed
        if (collisionPair.manifold == nullptr) {
            collisionPair.manifold = new Manifold(this, bodyA, bodyB, -1);
        }

        // save and merge old contacts // TODO create a perminant memory location for copying
        auto oldRA = collisionPair.manifold->getRA();
        auto oldRB = collisionPair.manifold->getRB();
        auto oldNormal = collisionPair.manifold->getNormal();
        auto oldJAn = collisionPair.manifold->getJAn();
        auto oldJBn = collisionPair.manifold->getJBn();
        auto oldJAt = collisionPair.manifold->getJAt();
        auto oldJBt = collisionPair.manifold->getJBt();
        auto oldC0 = collisionPair.manifold->getC0();
        auto oldPenalty = collisionPair.manifold->getPenalty();
        auto oldLambda = collisionPair.manifold->getLambda();
        auto oldStick = collisionPair.manifold->getStick();

        collisionPair.manifold->getNormal() = normal;
        sat(a, b, collisionPair);

        for (std::size_t i = 0; i < 2; i++) {
            if (glm::all(glm::epsilonEqual(oldRA[i], collisionPair.manifold->getRA()[i], 1e-10f))) {
                collisionPair.manifold->getPenalty()[i + JN] = oldPenalty[i + JN];
                collisionPair.manifold->getPenalty()[i + JT] = oldPenalty[i + JT];
                collisionPair.manifold->getLambda()[i + JN] = oldLambda[i + JN];
                collisionPair.manifold->getLambda()[i + JT] = oldLambda[i + JT];

                collisionPair.manifold->getStick()[i] = oldStick[i];
                if (oldStick[i]) {
                    collisionPair.manifold->getRA()[i] = oldRA[i];
                    collisionPair.manifold->getRB()[i] = oldRB[i];
                }
            }
        }
    }
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