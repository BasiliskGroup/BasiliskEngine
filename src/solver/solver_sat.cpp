#include "solver/physics.h"

// TODO SAT, we are doing the dumb solution right now (Not SAT)
void Solver::sat(ColliderRow& a, ColliderRow& b, CollisionPair& pair) {
    // select the deepest points 
    float firstDepth  = -std::numeric_limits<float>::infinity();
    float secondDepth = -std::numeric_limits<float>::infinity();
    int firstIndex    = -1;
    int secondIndex   = -1;

    for (uint i = 0; i < a.length; i++) {
        float depth = glm::dot(a.start[i], pair.dir);

        if (depth > firstDepth) {
            // shift current best to second
            secondDepth = firstDepth;
            secondIndex = firstIndex;

            firstDepth = depth;
            firstIndex = i;
        } else if (depth > secondDepth) {
            secondDepth = depth;
            secondIndex = i;
        }
    }

    // select vertices to use
    vec2 rA1 = a.start[firstIndex];
    vec2 rA2 = secondDepth - firstDepth > COLLISION_MARGIN ? a.start[secondIndex] : a.start[firstIndex];

    // write contact points to manifold index
    getManifoldTable()->getRA()[pair.manifoldIndex][0] = rA1 * a.scale;
    getManifoldTable()->getRA()[pair.manifoldIndex][1] = rA2 * a.scale;

    // ########################################################################################

    // select the deepest points 
    firstDepth  = std::numeric_limits<float>::infinity();
    secondDepth = std::numeric_limits<float>::infinity();
    firstIndex  = -1;
    secondIndex = -1;

    for (uint i = 0; i < b.length; i++) {
        float depth = glm::dot(b.start[i], pair.dir);

        if (depth < firstDepth) {
            // shift current best to second
            secondDepth = firstDepth;
            secondIndex = firstIndex;

            firstDepth = depth;
            firstIndex = i;
        } else if (depth < secondDepth) {
            secondDepth = depth;
            secondIndex = i;
        }
    }

    // select vertices to use
    vec2 rB1 = b.start[firstIndex];
    vec2 rB2 = secondDepth - firstDepth > COLLISION_MARGIN ? b.start[secondIndex] : b.start[firstIndex];

    // write contact points to manifold index
    getManifoldTable()->getRB()[pair.manifoldIndex][0] = rB1 * b.scale;
    getManifoldTable()->getRB()[pair.manifoldIndex][1] = rB2 * b.scale;
}

void Solver::intersect(ColliderRow& a, ColliderRow& b, CollisionPair& pair, const vec2& mtv) {

}

void Solver::clampToEdge(const vec2& edge, vec2& toClamp) {

}

void Solver::dotEdgeIntersect(const vec2* verts, uint start, Dots dots, float thresh) {

}

template <typename Compare>
void Solver::findBounds(Dots dots, const float thresh, uint& begin, uint& end, Compare cmp) {
    uint l = dots.size();

    // used for check variables
    bool isIn = cmp(dots[0], thresh);
}

template <typename Compare>
void Solver::findExtremes(Dots dots, uint& begin, uint& end, Compare cmp) {
    
}