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
    // compactBodies();
    if (PRINT_TIME) printDurationUS(beforeStep, timeNow(), "Body Compact:\t\t");

    auto beforeTransform = timeNow();
    bodyTable->computeTransforms();
    if (PRINT_TIME) printDurationUS(beforeTransform, timeNow(), "Body Transform:\t\t");

    // NOTE bodies are compact after this point

    auto beforeBroad = timeNow();
    sphericalCollision(); // broad collision TODO replace with BVH
    // // std::cout << "Broad Pairs:\t\t" << collisionPairs.size() << std::endl;
    if (PRINT_TIME) printDurationUS(beforeBroad, timeNow(), "Broad Collision:\t");

    // delete all manifolds TODO remove this when adding persistence
    // Collect manifolds to delete
    std::vector<Force*> toDelete;
    Force* force = forces;
    while (force != nullptr) {
        if (force->getType() == MANIFOLD) {
            toDelete.push_back(force);
            // Skip twin if it's next in the list
            Force* twin = force->getTwin();
            if (twin != nullptr && force->getNext() == twin) {
                force = twin->getNext();
                continue;
            }
        }
        force = force->getNext();
    }

    // Delete collected manifolds
    for (Force* f : toDelete) {
        delete f;
    }

    auto beforeNarrow = timeNow();
    narrowCollision(); // -> uncompacts manifolds
    if (PRINT_TIME) printDurationUS(beforeNarrow, timeNow(), "Narrow Collision:\t");

    // warmstart forces -> uncompacts forces
    auto beforeCompact = timeNow();
    compactForces();
    if (PRINT_TIME) printDurationUS(beforeCompact, timeNow(), "Force Compact:\t\t");

    // std::cout << "=== WARMSTART MANIFOLDS ===" << std::endl;
    for (Force* force = forces; force != nullptr; force = force->getNext()) {
        // std::cout << "Force: " << force << std::endl;
        
        // Debug print manifold data (similar to old version)
        if (force->getType() == MANIFOLD) {
            Manifold* m = static_cast<Manifold*>(force);
            // std::cout << "Manifold (bodyA=" << m->getBodyA() << ", bodyB=" << m->getBodyB() << ")" << std::endl;
            // std::cout << "  numContacts: " << 2 << std::endl;
            
            for (int c = 0; c < 2; c++) {
                // std::cout << "  Contact " << c << ":" << std::endl;
                // std::cout << "    normal: " << m->getNormal().x << ", " << m->getNormal().y << std::endl;
                // std::cout << "    rA: " << m->getRA()[c].x << ", " << m->getRA()[c].y << std::endl;
                // std::cout << "    rB: " << m->getRB()[c].x << ", " << m->getRB()[c].y << std::endl;
                // std::cout << "    JAn: " << m->J()[2 * c + JN].x << ", " << m->J()[2 * c + JN].y << ", " << m->J()[2 * c + JN].z << std::endl;
                // std::cout << "    JAt: " << m->J()[2 * c + JT].x << ", " << m->J()[2 * c + JT].y << ", " << m->J()[2 * c + JT].z << std::endl;
                // std::cout << "    C0: " << m->getC0()[c].x << ", " << m->getC0()[c].y << std::endl;
            }
        }
        
        // all manifolds here are valid
        force->initialize();
        
        for (uint i = 0; i < force->rows(); i++) {
            force->lambda()[i] = force->lambda()[i] * alpha * gamma;
            force->penalty()[i] = glm::clamp(force->penalty()[i] * gamma, PENALTY_MIN, PENALTY_MAX);
            force->penalty()[i] = glm::min(force->penalty()[i], force->stiffness()[i]);
        }
    }

    // warmstart bodies
    // std::cout << "=== WARMSTART BODIES ===" << std::endl;
    for (Rigid* body = bodies; body != nullptr; body = body->getNext()) {
        // std::cout << "Body: " << body << std::endl;
        // std::cout << "  mass: " << body->getMass() << ", moment: " << body->getMoment() << std::endl;
        
        // not clamping rotation bc I'm not a coward
        body->getInertial() = body->getPos() + body->getVel() * dt;
        if (body->getMass() > 0) body->getInertial() += glm::vec3(0, gravity, 0) * (dt * dt);

        // Adaptive warmstart (See original VBD paper)
        glm::vec3 accel = (body->getVel() - body->getPrevVel()) / dt;
        float accelExt = accel.y * (gravity > 0 ? 1 : -1);
        float accelWeight = glm::clamp(accelExt / abs(gravity), 0.0f, 1.0f);
        if (std::isinf(accelWeight)) accelWeight = 0.0f;

        // std::cout << "  accelWeight: " << accelWeight << ", accelExt: " << accelExt << std::endl;

        // Save initial position (x-) and compute warmstarted position (See original VBD paper)
        body->getInitial() = body->getPos();
        body->getPos() = body->getPos() + body->getVel() * dt + glm::vec3(0, gravity, 0) * (accelWeight * dt * dt);
        
        // std::cout << "  pos: " << body->getPos().x << ", " << body->getPos().y << ", " << body->getPos().z << std::endl;
        // std::cout << "  inertial: " << body->getInertial().x << ", " << body->getInertial().y << ", " << body->getInertial().z << std::endl;
    }

    // main solver loop
    for (uint it = 0; it < iterations; it++) {
        // std::cout << "=== ITERATION " << it << " ===" << std::endl;
        
        // PRIMAL UPDATE
        for (Rigid* body = bodies; body != 0; body = body->getNext()) {
            if (body->getMass() < 0) continue;

            glm::mat3x3 M = glm::diagonal3x3(glm::vec3{ body->getMass(), body->getMass(), body->getMoment() });
            glm::mat3x3 lhs = M / (dt * dt);
            glm::vec3 rhs = M / (dt * dt) * (body->getPos() - body->getInertial());

            // std::cout << "Body " << body << ":" << std::endl;
            // std::cout << "  pos: " << body->getPos().x << ", " << body->getPos().y << ", " << body->getPos().z << std::endl;
            // std::cout << "  inertial: " << body->getInertial().x << ", " << body->getInertial().y << ", " << body->getInertial().z << std::endl;
            // std::cout << "  initial rhs: " << rhs.x << ", " << rhs.y << ", " << rhs.z << std::endl;

            int forceCount = 0;
            // Iterate over all forces acting on the body
            for (Force* force = body->getForces(); force != nullptr; force = force->getNextA()) {
                // Compute constraint and its derivatives
                force->computeConstraint(alpha);
                force->computeDerivatives(body);

                for (uint i = 0; i < force->rows(); i++) {
                    // Use lambda as 0 if it's not a hard constraint
                    float lambda = std::isinf(force->stiffness()[i]) ? force->lambda()[i] : 0.0f;

                    // Compute the clamped force magnitude (Sec 3.2)
                    float f = glm::clamp(force->penalty()[i] * force->C()[i] + lambda, force->fmin()[i], force->fmax()[i]);

                    // std::cout << "  Force " << force << " row " << i << ":" << std::endl;
                    // std::cout << "    C: " << force->C()[i] << std::endl;
                    // std::cout << "    penalty: " << force->penalty()[i] << std::endl;
                    // std::cout << "    lambda: " << lambda << std::endl;
                    // std::cout << "    fmin: " << force->fmin()[i] << ", fmax: " << force->fmax()[i] << std::endl;
                    // std::cout << "    force: " << f << std::endl;
                    // std::cout << "    J: " << force->J()[i].x << ", " << force->J()[i].y << ", " << force->J()[i].z << std::endl;

                    // Compute the diagonally lumped geometric stiffness term (Sec 3.5)
                    glm::mat3x3 G = glm::diagonal3x3(glm::vec3(
                        glm::length(force->H()[i][0]), 
                        glm::length(force->H()[i][1]), 
                        glm::length(force->H()[i][2])
                    )) * abs(f);  // IMPORTANT: Should multiply by abs(f)!

                    // Accumulate force (Eq. 13) and hessian (Eq. 17)
                    rhs += force->J()[i] * f;
                    lhs += outerProduct(force->J()[i], force->J()[i] * force->penalty()[i]) + G;
                    
                    // std::cout << "    rhs after: " << rhs.x << ", " << rhs.y << ", " << rhs.z << std::endl;
                    forceCount++;
                }   
            }

            glm::vec3 x;
            solve(lhs, x, rhs);
            
            // std::cout << "  Final rhs: " << rhs.x << ", " << rhs.y << ", " << rhs.z << std::endl;
            // std::cout << "  Correction: " << x.x << ", " << x.y << ", " << x.z << std::endl;
            // std::cout << "  Total forces processed: " << forceCount << std::endl;
            
            body->getPos() -= x;
        }

        // DUAL UPDATE
        for (Force* force = forces; force != nullptr; force = force->getNext()) {
            force->computeConstraint(alpha);

            for (uint i = 0; i < force->rows(); i++) {
                // Use lambda as 0 if it's not a hard constraint
                float lambda = std::isinf(force->stiffness()[i]) ? force->lambda()[i] : 0.0f;

                // Update lambda (Eq 11)
                force->lambda()[i] = glm::clamp(force->penalty()[i] * force->C()[i] + lambda, force->fmin()[i], force->fmax()[i]);

                // TODO add fracture

                // update penalty
                if (force->lambda()[i] > force->fmin()[i] && force->lambda()[i] < force->fmax()[i]) {
                    force->penalty()[i] = glm::min(force->penalty()[i] + beta * abs(force->C()[i]), glm::min(PENALTY_MAX, force->stiffness()[i]));
                }
            }
        }
        
        // Update velocities at the end of second-to-last iteration
        if (it == iterations - 1) {
            // std::cout << "=== UPDATING VELOCITIES ===" << std::endl;
            for (Rigid* body = bodies; body != 0; body = body->getNext()) {
                body->getPrevVel() = body->getVel();
                if (body->getMass() > 0) {
                    body->getVel() = (body->getPos() - body->getInitial()) / dt;
                    // std::cout << "Body " << body << " new velocity: " << body->getVel().x << ", " << body->getVel().y << ", " << body->getVel().z << std::endl;
                }
            }
        }
    }

    auto beforeVel = timeNow();
    // updateVelocities(dt);  // This might be redundant now!
    if (PRINT_TIME) printDurationUS(beforeVel, timeNow(), "Velocities:\t\t");

    bodyTable->writeToNodes();
}

void Solver::compactBodies() {
    bodyTable->compact();

    // route forces back to their correct bodies after standard compact
    auto& bodyIndices = forceTable->getBodyIndex();
    auto& toDelete = forceTable->getToDelete();
    auto& inverseForceMap = bodyTable->getInverseForceMap();

    for (uint i = 0; i < forceTable->getSize(); i++) {
        // will index out of bounds
        if (toDelete[i] == true) {
            continue;
        }
        bodyIndices[i] = inverseForceMap[bodyIndices[i]];
    }
}

void Solver::setGravity(float gravity) {
    this->gravity = gravity;
}

void Solver::compactForces() {
    forceTable->compact();
}

// TODO replace this with BVH
void Solver::sphericalCollision() {
    auto& pos = bodyTable->getPos();
    auto& radii = bodyTable->getRadius();

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

            dpos = pos[i] - pos[j];
            radsum = radii[i] + radii[j];
            if (radsum * radsum > glm::length2(dpos)) {
                collisionPairs.emplace_back(i, j, manifold);
            }
        }
    }
}

void Solver::narrowCollision() {
    auto& manifoldNormals = getManifoldTable()->getNormal();
    auto& forcePointers = forceTable->getForces();
    auto& bodyPointers = bodyTable->getBodies();
    auto& specialIndices = forceTable->getSpecial();
    auto& types = forceTable->getType();
    auto& isA = forceTable->getIsA();

    // reserve space to perform all collisions 
    uint forceIndex, manifoldIndex;
    reserveForcesForCollision(forceIndex, manifoldIndex);

    int count = 0; // TODO debug variable

    ColliderRow a, b;
    CollisionPair collisionPair;
    for (const auto& pair : collisionPairs) {
        uint rowA = pair.bodyA;
        uint rowB = pair.bodyB;

        initColliderRow(rowA, manifoldIndex, a);
        initColliderRow(rowB, manifoldIndex, b);
        collisionPair.forceIndex = forceIndex;
        collisionPair.manifoldIndex = manifoldIndex;

        // determine if objects are colliding
        bool collided = gjk(a, b, collisionPair, 0);

        if (!collided) {
            // increment enumeration
            forceTable->markAsDeleted(forceIndex + 0);
            forceTable->markAsDeleted(forceIndex + 1);
            forceIndex += 2;
            manifoldIndex++;
            continue;
        }

        // determine collision normal
        count++;
        ushort frontIndex = epa(a, b, collisionPair);
        glm::vec2 normal = collisionPair.polytope[frontIndex].normal;

        // TODO determine if we need this check
        if (glm::length2(normal) < 1e-16f) {
            forceTable->markAsDeleted(forceIndex + 0);
            forceTable->markAsDeleted(forceIndex + 1);
            forceIndex += 2;
            manifoldIndex++;
            continue;
        }

        manifoldNormals[manifoldIndex] = glm::normalize(normal);
        if (glm::dot(normal, b.pos - a.pos) > 0) {
            manifoldNormals[manifoldIndex] *= -1;
        }

        // determine object overlap
        sat(a, b, collisionPair);

        // create manifold force in graph
        // TODO delay creation of these forces until after multithreading, these will insert into linked lists creating race conditions
        Manifold* aToB = new Manifold(this, bodyPointers[rowA], bodyPointers[rowB], forceIndex + 0); // A -> B
        Manifold* bToA = new Manifold(this, bodyPointers[rowB], bodyPointers[rowA], forceIndex + 1); // B -> A
        forcePointers[forceIndex + 0] = aToB;
        forcePointers[forceIndex + 1] = bToA;
        isA[forceIndex + 0] = true;
        isA[forceIndex + 1] = false;

        // set twins to allow for proper deletion
        aToB->getTwin() = bToA;
        bToA->getTwin() = aToB;

        // set special indices in each constructor, maybe set this ina constructor
        specialIndices[forceIndex + 0] = manifoldIndex;
        specialIndices[forceIndex + 1] = manifoldIndex;

        types[forceIndex + 0] = MANIFOLD;
        types[forceIndex + 1] = MANIFOLD;

        // increment enumeration
        forceIndex += 2;
        manifoldIndex++;
    }

    // // std::cout << "Positive Narrow:\t" << count << std::endl;
}

void Solver::reserveForcesForCollision(uint& forceIndex, uint& manifoldIndex) {
    auto& bodyIndices = forceTable->getBodyIndex();

    forceTable->reserveManifolds(collisionPairs.size(), forceIndex, manifoldIndex);

    // assign forces their bodies
    uint i = 0;
    for (const auto& pair : collisionPairs) {
        bodyIndices[forceIndex + i + 0] = pair.bodyA;
        bodyIndices[forceIndex + i + 1] = pair.bodyB;
        i += 2;
    }
}

void Solver::initColliderRow(uint row, uint manifoldIndex, ColliderRow& colliderRow) {
    colliderRow.pos = bodyTable->getPos()[row];
    colliderRow.scale = bodyTable->getScale()[row];
    colliderRow.mat = bodyTable->getMat()[row];
    colliderRow.imat = bodyTable->getIMat()[row];
    colliderRow.start = colliderTable->getStartPtr(bodyTable->getCollider()[row]);
    colliderRow.length = colliderTable->getLength(bodyTable->getCollider()[row]);
    colliderRow.simplex = getManifoldTable()->getSimplexPtr(manifoldIndex);
}

void Solver::warmstartManifolds() {
    getManifoldTable()->warmstart();

    auto& pos = bodyTable->getPos();
    auto& mat = bodyTable->getMat();
    auto& rAs = getManifoldTable()->getRA();
    auto& rBs = getManifoldTable()->getRB();
    auto& specials = forceTable->getSpecial();
    auto& bodyIndices = forceTable->getBodyIndex();
    auto& isA = forceTable->getIsA();
    auto& rAWs = getManifoldTable()->getRAW();
    auto& rBWs = getManifoldTable()->getRBW();

    for (uint i = 0; i < forceTable->getSize(); i++) {
        uint specialIndex = specials[i];
        uint bodyIndex = bodyIndices[i];

        if (isA[i]) {
            rAWs[specialIndex][0] = mat[bodyIndex] * rAs[specialIndex][0];
            rAWs[specialIndex][1] = mat[bodyIndex] * rAs[specialIndex][1];
        } else {
            rBWs[specialIndex][0] = mat[bodyIndex] * rBs[specialIndex][0];
            rBWs[specialIndex][1] = mat[bodyIndex] * rBs[specialIndex][1];
        }
    }

    auto& bases = getManifoldTable()->getBasis();
    auto& tangents = getManifoldTable()->getTangent();
    auto& normals = getManifoldTable()->getNormal();
    auto& C0s = getManifoldTable()->getC0();
    auto& Js = forceTable->getJ();

    // Set all C0 to zero
    for (uint i = 0; i < getManifoldTable()->getSize(); i++) {
        C0s[i] = BskVec2Pair();
    }

    if (DEBUG_MANIFOLD) print("=== WARMSTART MANIFOLDS ===");

    for (uint i = 0; i < forceTable->getSize(); i++) {
        uint specialIndex = specials[i];
        uint bodyIndex = bodyIndices[i];

        const glm::mat2x2& basis = bases[specialIndex];
        const glm::vec2& normal = normals[specialIndex];
        const glm::vec2& tangent = tangents[specialIndex];

        if (DEBUG_MANIFOLD) {
            print("Force " + std::to_string(i) + " (body " + std::to_string(bodyIndex) + ", " + 
                  (isA[i] ? "A" : "B") + ", manifold " + std::to_string(specialIndex) + ")");
            print("  normal: " + std::to_string(normal.x) + ", " + std::to_string(normal.y));
            print("  tangent: " + std::to_string(tangent.x) + ", " + std::to_string(tangent.y));
            print("  basis[0]: " + std::to_string(basis[0][0]) + ", " + std::to_string(basis[0][1]));
            print("  basis[1]: " + std::to_string(basis[1][0]) + ", " + std::to_string(basis[1][1]));
        }

        for (uint j = 0; j < 2; j++) {
            const glm::vec2& rW = isA[i] ? rAWs[specialIndex][j] : rBWs[specialIndex][j];
            float sign = 2 * isA[i] - 1; // TODO check if this negation is correct

            Js[i][2 * j + JN] = sign * glm::vec3{ basis[0][0], basis[0][1], cross(rW, normal) };
            Js[i][2 * j + JT] = sign * glm::vec3{ basis[1][0], basis[1][1], cross(rW, tangent) };

            glm::vec2 rcont = sign * basis * (xy(pos[bodyIndex]) + rW);
            C0s[specialIndex][j] += rcont;

            if (DEBUG_MANIFOLD) {
                print("  Contact " + std::to_string(j) + ":");
                print("    rW: " + std::to_string(rW.x) + ", " + std::to_string(rW.y));
                print("    sign: " + std::to_string(sign));
                print("    J_normal: " + std::to_string(Js[i][2*j+JN].x) + ", " + 
                      std::to_string(Js[i][2*j+JN].y) + ", " + std::to_string(Js[i][2*j+JN].z));
                print("    J_tangent: " + std::to_string(Js[i][2*j+JT].x) + ", " + 
                      std::to_string(Js[i][2*j+JT].y) + ", " + std::to_string(Js[i][2*j+JT].z));
                print("    C0 accumulated: " + std::to_string(C0s[specialIndex][j].x) + ", " + 
                      std::to_string(C0s[specialIndex][j].y));
                print("    rcont: " + std::to_string(rcont.x) + ", " + 
                          std::to_string(rcont.y));
            }
        }
    }
}

void Solver::warmstartForces() {
    forceTable->warmstart(alpha, gamma);
}

void Solver::warmstartBodies(float dt) {
    bodyTable->warmstartBodies(dt, gravity);
}

void Solver::updateVelocities(float dt) {
    bodyTable->updateVelocities(dt);
}

void Solver::mainloopPreload() {
    // set up all current dpX values since we do halfloads for compute constraints
    loadCdX(0, forceTable->getSize());
}

void Solver::primalUpdate(float dt) {
    auto& rhs = bodyTable->getRHS();
    auto& lhs = bodyTable->getLHS();
    auto& pos = bodyTable->getPos();
    auto& inertial = bodyTable->getInertial();
    auto& mass = bodyTable->getMass();
    auto& moment = bodyTable->getMoment();
    auto& bodyPtrs = bodyTable->getBodies();
    auto& lambdas = forceTable->getLambda();
    auto& stiffness = forceTable->getStiffness();
    auto& penalty = forceTable->getPenalty();
    auto& C = forceTable->getC();
    auto& fmax = forceTable->getFmax();
    auto& fmin = forceTable->getFmin();
    auto& J = forceTable->getJ();

    for (uint b = 0; b < bodyTable->getSize(); b++) {
        if (mass[b] <= 0) continue;

        lhs[b] = glm::diagonal3x3(glm::vec3{ mass[b], mass[b], moment[b] } / (dt * dt));
        rhs[b] = lhs[b] * (pos[b] - inertial[b]);

        if (DEBUG_PRIMAL) {
            print("Body " + std::to_string(b) + ":");
            print("  pos: " + std::to_string(pos[b].x) + ", " + std::to_string(pos[b].y) + ", " + std::to_string(pos[b].z));
            print("  inertial: " + std::to_string(inertial[b].x) + ", " + std::to_string(inertial[b].y) + ", " + std::to_string(inertial[b].z));
            print("  initial rhs: " + std::to_string(rhs[b].x) + ", " + std::to_string(rhs[b].y) + ", " + std::to_string(rhs[b].z));
        }

        Rigid* body = bodyPtrs[b];
        int forceCount = 0;
        for (Force* f = body->getForces(); f != nullptr; f = f->getNextA()) {
            uint forceIndex = f->getIndex();
            computeConstraints(forceIndex, forceIndex + 1, MANIFOLD);

            for (uint j = 0; j < MANIFOLD_ROWS; j++) {
                float lambda = glm::isinf(stiffness[forceIndex][j]) ? lambdas[forceIndex][j] : 0.0f;
                float force = glm::clamp(penalty[forceIndex][j] * C[forceIndex][j] + lambda, 
                                        fmin[forceIndex][j], fmax[forceIndex][j]);

                if (DEBUG_PRIMAL) {
                    print("  Force " + std::to_string(forceIndex) + " row " + std::to_string(j) + ":");
                    print("    C: " + std::to_string(C[forceIndex][j]));
                    print("    penalty: " + std::to_string(penalty[forceIndex][j]));
                    print("    lambda: " + std::to_string(lambda));
                    print("    fmin: " + std::to_string(fmin[forceIndex][j]) + ", fmax: " + std::to_string(fmax[forceIndex][j]));
                    print("    force: " + std::to_string(force));
                    print("    J: " + std::to_string(J[forceIndex][j].x) + ", " + 
                          std::to_string(J[forceIndex][j].y) + ", " + std::to_string(J[forceIndex][j].z));
                }

                rhs[b] += J[forceIndex][j] * force;
                lhs[b] += glm::outerProduct(J[forceIndex][j], J[forceIndex][j] * penalty[forceIndex][j]);
                
                if (DEBUG_PRIMAL) {
                    print("    rhs after: " + std::to_string(rhs[b].x) + ", " + 
                          std::to_string(rhs[b].y) + ", " + std::to_string(rhs[b].z));
                }
                forceCount++;
            }
        }

        glm::vec3 x = glm::vec3(0);
        solve(lhs[b], x, rhs[b]);

        if (DEBUG_PRIMAL) {
            print("  Final rhs: " + std::to_string(rhs[b].x) + ", " + std::to_string(rhs[b].y) + ", " + std::to_string(rhs[b].z));
            print("  Correction: " + std::to_string(x.x) + ", " + std::to_string(x.y) + ", " + std::to_string(x.z));
            print("  Total forces processed: " + std::to_string(forceCount));
        }

        pos[b] -= x;
    }
}

void Solver::dualUpdate(float dt) {
    auto& lambdas = forceTable->getLambda();
    auto& stiffness = forceTable->getStiffness();
    auto& penalty = forceTable->getPenalty();
    auto& fmax = forceTable->getFmax();
    auto& fmin = forceTable->getFmin();
    auto& C = forceTable->getC();

    for (uint forceIndex = 0; forceIndex < forceTable->getSize(); forceIndex++) {
        computeConstraints(forceIndex, forceIndex + 1, MANIFOLD);

        for (uint j = 0; j < MANIFOLD_ROWS; j++) {
            // Use lambda as 0 if it's not a hard constraint
            float lambda = glm::isinf(stiffness[forceIndex][j]) ? lambdas[forceIndex][j] : 0.0f;

            // Update lambda (Eq 11)
            // Note that we don't include non-conservative forces (ie motors) in the lambda update, as they are not part of the dual problem.
            float f = glm::clamp(penalty[forceIndex][j] * C[forceIndex][j] + lambda, fmin[forceIndex][j], fmax[forceIndex][j]);

            // TODO add fracture
    
            // Update the penalty parameter and clamp to material stiffness if we are within the force bounds (Eq. 16)
            if (lambdas[forceIndex][j] > fmin[forceIndex][j] && lambdas[forceIndex][j] < fmax[forceIndex][j]) {
                penalty[forceIndex][j] = glm::min(penalty[forceIndex][j] + beta * abs(C[forceIndex][j]), PENALTY_MAX, stiffness[forceIndex][j]);
            }
        }
    }
}

void Solver::computeConstraints(uint start, uint end, ushort type) {
    auto& pos =     bodyTable->getPos();
    auto& initial = bodyTable->getInitial();
    auto& J =            forceTable->getJ();
    auto& fmax =         forceTable->getFmax();
    auto& fmin =         forceTable->getFmin();
    auto& C =            forceTable->getC();
    auto& lambda =       forceTable->getLambda();
    auto& specialIndex = forceTable->getSpecial();
    auto& friction = getManifoldTable()->getFriction();
    auto& C0 =       getManifoldTable()->getC0();
    auto& cdA =      getManifoldTable()->getCdA();
    auto& cdB =      getManifoldTable()->getCdB();


    // other dp will already be loaded, only focus on self
    loadCdX(start, end);

    for (uint i = start; i < end; i++) {
        uint special = specialIndex[i];
        for (uint j = 0; j < 2; j++) {
            // Compute the Taylor series approximation of the constraint function C(x) (Sec 4)
            C[i][2 * j + JN] = C0[special][j].x * (1 - alpha) + cdA[special][2 * j + JN] + cdB[special][2 * j + JN];
            C[i][2 * j + JT] = C0[special][j].y * (1 - alpha) + cdA[special][2 * j + JT] + cdB[special][2 * j + JT];

            print("Computing C");
            print(cdA[special][2 * j + JN]);
            print(cdB[special][2 * j + JN]);
            print(cdA[special][2 * j + JT]);
            print(cdB[special][2 * j + JT]);

            float frictionBound = abs(lambda[i][2 * j] * friction[special]);
            fmax[i][2 * j + JT] = frictionBound;
            fmin[i][2 * j + JT] = -frictionBound;

            // TODO add force stick
        }
    }
}

void Solver::computeDerivatives(uint start, uint end, ushort type) {
    // Manifolds do not need to compute derivatives
    if (type == 0) {
        return;
    }

    for (uint i = start; i < end; i++) {

    }
}

void Solver::loadCdX(uint start, uint end) {
    auto& pos = bodyTable->getPos();
    auto& initial = bodyTable->getInitial();
    auto& bodyIndices = forceTable->getBodyIndex();
    auto& specialIndices = forceTable->getSpecial();
    auto& isA = forceTable->getIsA();
    auto& J = forceTable->getJ();
    auto& cdA = getManifoldTable()->getCdA();
    auto& cdB = getManifoldTable()->getCdB();

    for (uint i = start; i < end; i++) {
        uint special = specialIndices[i];
        uint body = bodyIndices[i];

        BskFloatROWS& cdX = isA[i] ? cdA[special] : cdB[special];

        for (ushort j = 0; j < 2; j++) {
            // print("Cds nuts " + std::to_string(i));
            // print(J[i][2 * j + JN]);
            // print(pos[body] - initial[body]);

            if (isA[i]) print("cdA");
            else print("cdB");

            glm::vec2 dpX = pos[body] - initial[body];

            print(std::to_string(J[i][2 * j + JN].x) + " " + std::to_string(J[i][2 * j + JN].y) + " " + std::to_string(dpX.x) + " " + std::to_string(dpX.y));
            print(std::to_string(J[i][2 * j + JT].x) + " " + std::to_string(J[i][2 * j + JT].y) + " " + std::to_string(dpX.x) + " " + std::to_string(dpX.y));

            cdX[2 * j + JN] = glm::dot(J[i][2 * j + JN], pos[body] - initial[body]);
            cdX[2 * j + JT] = glm::dot(J[i][2 * j + JT], pos[body] - initial[body]);

            // print(cdX[2 * j + JN]);
            // print(cdX[2 * j + JT]);
        }
    }
}

void Solver::draw() {
    // TODO draw everything
}

}