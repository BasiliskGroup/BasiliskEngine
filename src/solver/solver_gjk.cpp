#include "solver/physics.h"

#define GJK_PRINT false

bool Solver::gjk(ColliderRow& a, ColliderRow& b, CollisionPair& pair, uint freeIndex) {

    if (GJK_PRINT) print("\nNew GJK\n");
    if (GJK_PRINT) print("positions");
    if (GJK_PRINT) print(a.pos);
    if (GJK_PRINT) print(b.pos);

    for (uint _ = 0; _ < GJK_ITERATIONS; ++_) {
        // get next direction or test simplex if full
        freeIndex = handleSimplex(a, b, pair, freeIndex);
        if (GJK_PRINT) print("iteration: " + std::to_string(freeIndex));
        if (GJK_PRINT) print("dir");
        if (GJK_PRINT) print(pair.dir);

        // termination signal
        if (freeIndex == -1) {
            return true;
        }

        // get next support point
        addSupport(a, b, pair, freeIndex);
        if (GJK_PRINT) print("sp");
        if (GJK_PRINT) print(pair.minks[freeIndex]);

        // if the point we found didn't cross the origin, we are not colliding
        if (glm::dot(pair.minks[freeIndex], pair.dir) < COLLISION_MARGIN) {
            if (GJK_PRINT) print("out");
            if (GJK_PRINT) print(freeIndex);
            return false;
        }

        freeIndex++;
    }
    
    if (GJK_PRINT) print("time out");
    return false;
}

uint Solver::handleSimplex(ColliderRow& a, ColliderRow& b, CollisionPair& pair, uint freeIndex) {
    switch (freeIndex) {
        case 0: return handle0(a, b, pair);
        case 1: return handle1(a, b, pair);
        case 2: return handle2(a, b, pair);
        case 3: return handle3(a, b, pair);
        default: throw std::runtime_error("simplex has incorrect freeIndex");
    }
}

uint Solver::handle0(ColliderRow& a, ColliderRow& b, CollisionPair& pair) {
    pair.dir = b.pos - a.pos;

    // if center of masses are super close, they must be colliding
    // this also avoids numerical stability issues
    if (glm::length2(pair.dir) < COLLISION_MARGIN) {
        return -1;
    }

    return 0;
}

uint Solver::handle1(ColliderRow& a, ColliderRow& b, CollisionPair& pair) {
    pair.dir = -pair.minks[0];
    return 1;
}

uint Solver::handle2(ColliderRow& a, ColliderRow& b, CollisionPair& pair) {
    vec2 CB = pair.minks[1] - pair.minks[0];
    vec2 CO =               - pair.minks[0];
    tripleProduct(CB, CO, CB, pair.dir);

    if (glm::length2(pair.dir) < COLLISION_MARGIN) {
        // fallback perpendicular
        perpTowards(CB, CO, pair.dir);
    }

    return 2;
}

uint Solver::handle3(ColliderRow& a, ColliderRow& b, CollisionPair& pair) {
    vec2 AB = pair.minks[1] - pair.minks[2];
    vec2 AC = pair.minks[0] - pair.minks[2];
    vec2 AO =               - pair.minks[2];
    vec2 CO =               - pair.minks[0];

    vec2 perp;
    perpTowards(AB, CO, perp);
    if (glm::dot(perp, AO) > -COLLISION_MARGIN) {
        // remove 0
        a.simplex[0]    = a.simplex[2];
        b.simplex[0]    = b.simplex[2];
        pair.minks[0] = pair.minks[2];

        pair.dir = perp;
        return 2;
    }

    vec2 BO = -pair.minks[1];
    perpTowards(AC, BO, perp);
    if (glm::dot(perp, AO) > -COLLISION_MARGIN) {
        // remove 1
        a.simplex[1]    = a.simplex[2];
        b.simplex[1]    = b.simplex[2];
        pair.minks[1] = pair.minks[2];

        pair.dir = perp;
        return 2;
    }

    // we have found a collision
    return -1;
}

void Solver::addSupport(ColliderRow& a, ColliderRow& b, CollisionPair& pair, uint insertIndex) {
    // direct search vector into local space
    vec2 dirA = a.imat *  pair.dir;
    vec2 dirB = b.imat * -pair.dir;

    vec2 localA;
    vec2 localB;

    getFar(a, dirA, localA);
    getFar(b, dirB, localB);

    // Transform selected local vertices into world space
    vec2 worldA = a.pos + a.mat * localA;
    vec2 worldB = b.pos + b.mat * localB;

    if (GJK_PRINT) print("matrices (CW positive)");
    if (GJK_PRINT) print(a.mat);
    if (GJK_PRINT) print(b.mat);
    if (GJK_PRINT) print(a.imat);
    if (GJK_PRINT) print(b.imat);

    if (GJK_PRINT) print("worlds");
    if (GJK_PRINT) print(worldA);
    if (GJK_PRINT) print(worldB);

    // Compute Minkowski support point
    pair.minks[insertIndex] = worldA - worldB;
}

void Solver::getFar(const ColliderRow& row, const vec2& dir, vec2& simplexLocal) {
    uint farIndex = 0;
    float maxDot = glm::dot(row.start[0], dir);

    if (GJK_PRINT) print(dir);
    
    for (uint i = 0; i < row.length; ++i) {
        float d = glm::dot(row.start[i], dir);
        if (GJK_PRINT) print("  vert[" + std::to_string(i) + "]: <" + std::to_string(row.start[i][0]) + ", " + std::to_string(row.start[i][1]) + "> dot: " + std::to_string(d));
        if (d > maxDot) {
            maxDot = d;
            farIndex = i;
        }
    }
    
    if (GJK_PRINT) print("Selected vertex " + std::to_string(farIndex));
    simplexLocal = row.start[farIndex];
}