#include <basilisk/solver/physics.h>
#include "clipper2/clipper.h"
using namespace Clipper2Lib;

namespace bsk::internal {

// TODO SAT, we are doing the dumb solution right now (Not SAT)
void Solver::sat(ColliderRow& a, ColliderRow& b, CollisionPair& pair)
{
    // Convert model->world (already in your file)
    auto toWorld = [&](const ColliderRow& row, const glm::vec2& v) {
        return row.mat * v + row.pos;
    };

    // Convert world -> collider local scaled (already in your file)
    auto worldToScaledLocal = [&](const ColliderRow& row, const glm::vec2& pw) {
        return row.imat * (pw - row.pos) * row.scale;
    };

    // Build world-space polygons for A and B (CCW assumption)
    std::vector<glm::vec2> polyA(a.length), polyB(b.length);
    for (uint i = 0; i < a.length; ++i)
        polyA[i] = toWorld(a, a.start[i]);
    for (uint i = 0; i < b.length; ++i)
        polyB[i] = toWorld(b, b.start[i]);

    // --- Prepare Clipper2 integer paths ---
    constexpr double CLIP_SCALE = 1e6;
    auto toPoint64 = [&](const glm::vec2 &p) -> Point64 {
        return Point64{ llround(p.x * CLIP_SCALE), llround(p.y * CLIP_SCALE) };
    };

    Path64 subj, clip;
    subj.reserve(polyA.size());
    clip.reserve(polyB.size());
    for (auto &p : polyA) subj.push_back(toPoint64(p));
    for (auto &p : polyB) clip.push_back(toPoint64(p));

    Paths64 subj_paths{ subj };
    Paths64 clip_paths{ clip };

    // --- Intersect using Clipper2 ---
    Paths64 solution = Intersect(subj_paths, clip_paths, FillRule::NonZero);

    // If intersection empty -> fallback to midpoint contact
    if (solution.empty())
    {
        glm::vec2 cworld = (polyA[0] + polyB[0]) * 0.5f;
        glm::vec2 ca = a.imat * (cworld - a.pos);
        glm::vec2 cb = b.imat * (cworld - b.pos);

        pair.manifold->getRA()[0] = ca * a.scale;
        pair.manifold->getRA()[1] = ca * a.scale;
        pair.manifold->getRB()[0] = cb * b.scale;
        pair.manifold->getRB()[1] = cb * b.scale;
        return;
    }

    // --- Find extreme vertices along pair.dir (world-space) ---
    glm::vec2 dir = pair.dir;
    if (glm::length(dir) > 0.0f) dir = glm::normalize(dir);

    float bestA = -std::numeric_limits<float>::infinity();
    float bestB =  std::numeric_limits<float>::infinity();

    glm::vec2 Apts[2]; int Acnt = 0;
    glm::vec2 Bpts[2]; int Bcnt = 0;

    constexpr float EPS_WORLD = 1e-4f;

    // Iterate every polygon and vertex in the solution
    for (const Path64 &path : solution)
    {
        for (const Point64 &pt64 : path)
        {
            glm::vec2 pworld = glm::vec2(static_cast<float>(pt64.x) / static_cast<float>(CLIP_SCALE),
                                         static_cast<float>(pt64.y) / static_cast<float>(CLIP_SCALE));

            float s = glm::dot(pworld, dir);

            // update max (A)
            if (s > bestA + EPS_WORLD) {
                bestA = s;
                Apts[0] = pworld;
                Acnt = 1;
            } else if (fabsf(s - bestA) <= EPS_WORLD) {
                if (Acnt < 2) Apts[Acnt++] = pworld;
            }

            // update min (B)
            if (s < bestB - EPS_WORLD) {
                bestB = s;
                Bpts[0] = pworld;
                Bcnt = 1;
            } else if (fabsf(s - bestB) <= EPS_WORLD) {
                if (Bcnt < 2) Bpts[Bcnt++] = pworld;
            }
        }
    }

    // Ensure duplicates if single point found
    if (Acnt == 1) { Apts[1] = Apts[0]; Acnt = 2; }
    if (Bcnt == 1) { Bpts[1] = Bpts[0]; Bcnt = 2; }

    // Convert selected world points into collider-local *scaled* coordinates
    glm::vec2 a0 = worldToScaledLocal(a, Apts[0]);
    glm::vec2 a1 = worldToScaledLocal(a, Apts[1]);
    glm::vec2 b0 = worldToScaledLocal(b, Bpts[0]);
    glm::vec2 b1 = worldToScaledLocal(b, Bpts[1]);

    // Store into manifold
    pair.manifold->getRA()[0] = a0;
    pair.manifold->getRA()[1] = a1;
    pair.manifold->getRB()[0] = b0;
    pair.manifold->getRB()[1] = b1;
}

void Solver::intersect(ColliderRow& a, ColliderRow& b, CollisionPair& pair, const glm::vec2& mtv) {

}

void Solver::clampToEdge(const glm::vec2& edge, glm::vec2& toClamp) {

}

void Solver::dotEdgeIntersect(const glm::vec2* verts, uint start, Dots dots, float thresh) {

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

}