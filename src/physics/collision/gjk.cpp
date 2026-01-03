#include <basilisk/physics/collision/gjk.h>
#include <basilisk/physics/rigid.h>
#include <basilisk/physics/maths.h>
#include <basilisk/util/maths.h>
#include <basilisk/physics/collision/collider.h>

namespace bsk::internal {

bool gjk(Rigid* bodyA, Rigid* bodyB, CollisionPair& pair) {
    std::size_t freeIndex = 0;
    for (std::size_t i = 0; i < GJK_ITERATIONS; ++i) {
        // get next direction or test simplex if full
        freeIndex = handleSimplex(bodyA, bodyB, pair, freeIndex);

        // termination signal
        if (freeIndex == -1) {
            return true;
        }

        // get next support point
        addSupport(bodyA, bodyB, pair, freeIndex);

        // if the point we found didn't cross the origin, we are not colliding
        if (glm::dot(pair.simplex[freeIndex], pair.searchDir) < EPSILON) {
            return false;
        }

        freeIndex++;
    }

    return false;
}

std::size_t handleSimplex(Rigid* bodyA, Rigid* bodyB, CollisionPair& pair, std::size_t freeIndex) {
    switch (freeIndex) {
        case 0: return handle0(bodyA, bodyB, pair);
        case 1: return handle1(bodyA, bodyB, pair);
        case 2: return handle2(bodyA, bodyB, pair);
        case 3: return handle3(bodyA, bodyB, pair);
        default: throw std::runtime_error("simplex has incorrect freeIndex");
    }
}

std::size_t handle0(Rigid* bodyA, Rigid* bodyB, CollisionPair& pair) {
    pair.searchDir = glm::vec2(bodyB->getPosition().x, bodyB->getPosition().y) - glm::vec2(bodyA->getPosition().x, bodyA->getPosition().y);
    return 0;
}

std::size_t handle1(Rigid* bodyA, Rigid* bodyB, CollisionPair& pair) {
    pair.searchDir = -pair.simplex[0];
    return 1;
}

std::size_t handle2(Rigid* bodyA, Rigid* bodyB, CollisionPair& pair) {
    glm::vec2 CB = pair.simplex[1] - pair.simplex[0];
    glm::vec2 CO =               - pair.simplex[0];
    tripleProduct(CB, CO, CB, pair.searchDir);

    if (glm::length2(pair.searchDir) < EPSILON) {
        // fallback perpendicular
        perpTowards(CB, CO, pair.searchDir);
    }

    return 2;
}

std::size_t handle3(Rigid* bodyA, Rigid* bodyB, CollisionPair& pair) {
    glm::vec2 AB = pair.simplex[1] - pair.simplex[2];
    glm::vec2 AC = pair.simplex[0] - pair.simplex[2];
    glm::vec2 AO =               - pair.simplex[2];
    glm::vec2 CO =               - pair.simplex[0];

    // remove 0
    glm::vec2 perp;
    perpTowards(AB, CO, perp);
    if (glm::dot(perp, AO) > -EPSILON) {
        pair.simplex[0] = pair.simplex[2];
        pair.searchDir = perp;
        return 2;
    }

    // remove 1
    glm::vec2 BO = -pair.simplex[1];
    perpTowards(AC, BO, perp);
    if (glm::dot(perp, AO) > -EPSILON) {
        pair.simplex[1] = pair.simplex[2];
        pair.searchDir = perp;
        return 2;
    }

    // we have found a collision
    return -1;
}

void addSupport(Rigid* bodyA, Rigid* bodyB, CollisionPair& pair, std::size_t insertIndex) {
    // Step 1: Transform search direction from world space to each body's local space
    // The search direction is in Minkowski space (worldA - worldB), so we need to find
    // the furthest vertex in each body's local coordinate system along this direction.
    // For bodyA, we search in the direction of searchDir.
    // For bodyB, we search in the opposite direction (-searchDir) since we're computing A - B.
    
    // Create rotation matrices for both bodies (position.z contains the rotation angle)
    glm::vec3 posA = bodyA->getPosition();
    glm::vec3 posB = bodyB->getPosition();
    glm::mat2 rotA = rotation(posA.z);
    glm::mat2 rotB = rotation(posB.z);
    
    // Inverse rotation matrix = transpose for 2D rotation matrices
    glm::mat2 invRotA = transpose(rotA);
    glm::mat2 invRotB = transpose(rotB);
    
    // Transform search direction to local space
    glm::vec2 dirA = invRotA * pair.searchDir;  // Transform searchDir to bodyA's local space
    glm::vec2 dirB = invRotB * (-pair.searchDir); // Transform -searchDir to bodyB's local space

    // Step 2: Find the furthest vertex in local space along the transformed direction
    glm::vec2 localA;
    glm::vec2 localB;
    getFar(bodyA, dirA, localA);
    getFar(bodyB, dirB, localB);
    
    // Step 3: Transform the selected local vertices back to world space
    // World position = rotation * local_vertex + translation
    glm::vec2 worldA = rotA * localA + glm::vec2(posA.x, posA.y);
    glm::vec2 worldB = rotB * localB + glm::vec2(posB.x, posB.y);

    // Step 4: Compute Minkowski difference point (A - B) for the simplex
    pair.simplex[insertIndex] = worldA - worldB;
}

void getFar(const Rigid* body, const glm::vec2 &dir, glm::vec2 &simplexLocal) {
    std::size_t farIndex = 0;
    Collider* collider = body->getCollider();
    float maxDot = glm::dot(collider->getVertices()[0], dir);
    for (std::size_t i = 0; i < collider->getVertices().size(); ++i) {
        float d = glm::dot(collider->getVertices()[i], dir);
        if (d > maxDot) {
            maxDot = d;
            farIndex = i;
        }
    }
    simplexLocal = collider->getVertices()[farIndex];
}
}