#include <basilisk/solver/physics.h>

namespace bsk::internal {

// TODO SAT, we are doing the dumb solution right now (Not SAT)
void Solver::sat(ColliderRow& a, ColliderRow& b, CollisionPair& pair)
{
    // Convert model->world
    auto toWorld = [&](const ColliderRow& row, const glm::vec2& v_model_scaled) {
        // row.start[i] is model-space, multiply by scale to get local,
        // row.mat transforms to world, then + pos.
        return row.mat * (v_model_scaled) + row.pos;
    };

    // Sutherland-Hodgman clipping against a single plane:
    auto clipPlane = [&](const std::vector<glm::vec2>& poly,
                         std::vector<glm::vec2>& out,
                         const glm::vec2& n,
                         float d)
    {
        out.clear();
        if (poly.empty()) return;

        auto side = [&](const glm::vec2& p) { return glm::dot(n,p) - d; };

        for (size_t i = 0; i < poly.size(); i++)
        {
            const glm::vec2& A = poly[i];
            const glm::vec2& B = poly[(i+1)%poly.size()];
            float aSide = side(A);
            float bSide = side(B);

            bool Ain = aSide <= 0.f;
            bool Bin = bSide <= 0.f;

            if (Ain && Bin)
            {
                out.push_back(B);
            }
            else if (Ain && !Bin)
            {
                float t = aSide / (aSide - bSide);
                out.push_back(A + t*(B-A));
            }
            else if (!Ain && Bin)
            {
                float t = aSide / (aSide - bSide);
                out.push_back(A + t*(B-A));
                out.push_back(B);
            }
        }
    };

    // ---- 1) Build world-space polygons ----
    std::vector<glm::vec2> polyA(a.length), polyB(b.length);
    for (uint i=0; i<a.length; i++)
        polyA[i] = toWorld(a, a.start[i] * a.scale);

    for (uint i=0; i<b.length; i++)
        polyB[i] = toWorld(b, b.start[i] * b.scale);

    // ---- 2) Intersect A ∩ B using Sutherland-Hodgman ----
    // Start with A, clip against all planes of B.

    std::vector<glm::vec2> cur = polyA;
    std::vector<glm::vec2> next;

    for (uint i=0; i<b.length; i++)
    {
        glm::vec2 p0 = polyB[i];
        glm::vec2 p1 = polyB[(i+1)%b.length];
        glm::vec2 edge = p1 - p0;

        // outward normal of CCW polygon
        glm::vec2 n(edge.y, -edge.x);
        n = glm::normalize(n);

        float d = glm::dot(n, p0); // plane: dot(n,x) <= d is INSIDE

        clipPlane(cur, next, n, d);
        cur = next;

        if (cur.empty()) break;
    }

    // If the intersection polygon is empty, we still give some contact fallback
    if (cur.empty())
    {
        glm::vec2 cworld = (toWorld(a,a.start[0]*a.scale) + toWorld(b,b.start[0]*b.scale)) * 0.5f;
        glm::vec2 ca = a.imat * (cworld - a.pos);
        glm::vec2 cb = b.imat * (cworld - b.pos);

        getManifoldTable()->getRA()[pair.manifoldIndex][0] = ca;
        getManifoldTable()->getRA()[pair.manifoldIndex][1] = ca;
        getManifoldTable()->getRB()[pair.manifoldIndex][0] = cb;
        getManifoldTable()->getRB()[pair.manifoldIndex][1] = cb;
        return;
    }

    // ---- 3) Find furthest points along ±pair.dir ----
    glm::vec2 dir = pair.dir;
    float bestA = -std::numeric_limits<float>::infinity();
    float bestB =  std::numeric_limits<float>::infinity();

    glm::vec2 Apts[2]; int Acnt = 0;
    glm::vec2 Bpts[2]; int Bcnt = 0;

    for (auto& p : cur)
    {
        float s = glm::dot(p, dir);
        // A wants max(s)
        if (s > bestA + 1e-6f) {
            bestA = s;
            Apts[0] = p;
            Acnt = 1;
        } else if (fabs(s - bestA) < 1e-6f) {
            if (Acnt < 2) Apts[Acnt++] = p;
        }

        // B wants min(s)
        if (s < bestB - 1e-6f) {
            bestB = s;
            Bpts[0] = p;
            Bcnt = 1;
        } else if (fabs(s - bestB) < 1e-6f) {
            if (Bcnt < 2) Bpts[Bcnt++] = p;
        }
    }

    // if only one point, duplicate it
    if (Acnt == 1) { Apts[1] = Apts[0]; Acnt = 2; }
    if (Bcnt == 1) { Bpts[1] = Bpts[0]; Bcnt = 2; }

    // ---- 4) Convert world → collider local scaled (your manifold representation) ----
    auto worldToScaledLocal = [&](const ColliderRow& row, const glm::vec2& pw) {
        return row.imat * (pw - row.pos);
    };

    glm::vec2 a0 = worldToScaledLocal(a, Apts[0]);
    glm::vec2 a1 = worldToScaledLocal(a, Apts[1]);
    glm::vec2 b0 = worldToScaledLocal(b, Bpts[0]);
    glm::vec2 b1 = worldToScaledLocal(b, Bpts[1]);

    // Your code stored RA[0], RA[1] and RB[1], RB[0]
    getManifoldTable()->getRA()[pair.manifoldIndex][0] = a0 * a.scale;
    getManifoldTable()->getRA()[pair.manifoldIndex][1] = a1 * a.scale;
    getManifoldTable()->getRB()[pair.manifoldIndex][1] = b0 * b.scale;
    getManifoldTable()->getRB()[pair.manifoldIndex][0] = b1 * b.scale;
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

// void Solver::sat(ColliderRow& a, ColliderRow& b, CollisionPair& pair) {
//     // Scratch buffers
//     std::array<uint, 8> idcs{};
//     std::array<glm::vec2, 8> world{};

//     // Compute projections along MTV
//     glm::vec2 mtv = pair.dir; // MTV from EPA/GJK, normalized
//     std::vector<float> dots_a(a.length);
//     std::vector<float> dots_b(b.length);

//     for (uint i = 0; i < a.length; i++)
//         dots_a[i] = glm::dot(a.start[i], mtv);

//     for (uint i = 0; i < b.length; i++)
//         dots_b[i] = glm::dot(b.start[i], mtv);

//     // -----------------------
//     // INTERSECT REFERENCE A
//     // -----------------------
//     // Find extreme points
//     float max_a = -std::numeric_limits<float>::infinity();
//     float min_b = std::numeric_limits<float>::infinity();
//     idcs[0] = idcs[1] = 0;
//     idcs[4] = idcs[5] = 0;

//     // Max of A
//     for (uint i = 0; i < a.length; i++) {
//         if (dots_a[i] > max_a + COLLISION_MARGIN) {
//             max_a = dots_a[i];
//             idcs[0] = idcs[1] = i;
//         } else if (std::abs(dots_a[i] - max_a) <= COLLISION_MARGIN) {
//             idcs[1] = i;
//         }
//     }

//     // Min of B
//     for (uint i = 0; i < b.length; i++) {
//         if (dots_b[i] < min_b - COLLISION_MARGIN) {
//             min_b = dots_b[i];
//             idcs[4] = idcs[5] = i;
//         } else if (std::abs(dots_b[i] - min_b) <= COLLISION_MARGIN) {
//             idcs[5] = i;
//         }
//     }

//     // Compute bounds of reference edges along perp line
//     // (using hillclimb-style search on projection)
//     auto findBounds = [&](const std::vector<float>& dots, uint& start, uint& end, float thresh, bool isMax) {
//         bool inside = (isMax) ? dots[0] >= thresh : dots[0] <= thresh;
//         start = end = 0;
//         bool beginFound = false, endFound = false;

//         for (uint i = 1; i < dots.size(); i++) {
//             float c = dots[i];
//             if ((isMax && c >= thresh) || (!isMax && c <= thresh)) {
//                 if (!inside) {
//                     start = i - 1;
//                     inside = true;
//                     beginFound = true;
//                 }
//                 continue;
//             }
//             if (inside) {
//                 end = i - 1;
//                 inside = false;
//                 endFound = true;
//             }
//         }
//         if (!endFound) end = dots.size() - 1;
//         if (!beginFound) start = dots.size() - 1;
//     };

//     findBounds(dots_a, idcs[2], idcs[3], min_b, true);   // A edges
//     findBounds(dots_b, idcs[6], idcs[7], max_a, false);  // B edges

//     // -----------------------
//     // COMPUTE INTERSECTIONS
//     // -----------------------
//     auto dotEdgeIntersect = [&](uint start, glm::vec2* verts, uint len, float* dots, float thresh) -> glm::vec2 {
//         uint end = (start + 1 < len) ? start + 1 : 0;
//         float l1 = dots[start];
//         float l2 = dots[end];
//         float t = (thresh - l1) / (l2 - l1);
//         t = glm::clamp(t, 0.0f, 1.0f);
//         return verts[start] + t * (verts[end] - verts[start]);
//     };

//     auto edgeClamp = [](glm::vec2& p0, glm::vec2& p1, glm::vec2 e0, glm::vec2 e1) {
//         glm::vec2 v = e1 - e0;
//         float vv = glm::dot(v, v);
//         float t0 = glm::dot(p0 - e0, v) / vv;
//         float t1 = glm::dot(p1 - e0, v) / vv;
//         t0 = glm::clamp(t0, 0.0f, 1.0f);
//         t1 = glm::clamp(t1, 0.0f, 1.0f);
//         p0 = e0 + t0 * v;
//         p1 = e0 + t1 * v;
//     };

//     // Load world locations for extreme vertices
//     world[0] = a.start[idcs[0]];
//     world[1] = a.start[idcs[1]];
//     world[4] = b.start[idcs[4]];
//     world[5] = b.start[idcs[5]];

//     if (idcs[2] != idcs[3]) {
//         world[2] = dotEdgeIntersect(idcs[2], a.start, a.length, dots_a.data(), min_b);
//         world[3] = dotEdgeIntersect(idcs[3], a.start, a.length, dots_a.data(), min_b);
//     }

//     if (idcs[6] != idcs[7]) {
//         world[6] = dotEdgeIntersect(idcs[6], b.start, b.length, dots_b.data(), max_a);
//         world[7] = dotEdgeIntersect(idcs[7], b.start, b.length, dots_b.data(), max_a);
//     }


//     if (idcs[6] != idcs[7])
//         edgeClamp(world[6], world[7], world[0], world[1]);

//     if (idcs[2] != idcs[3])
//         edgeClamp(world[2], world[3], world[4], world[5]);

//     // -----------------------
//     // WRITE TO MANIFOLD
//     // -----------------------
//     auto inv_a = a.imat;
//     auto inv_b = b.imat;

//     // Transform world points back to local
//     glm::vec2 rA0 = inv_a * (world[0] * a.scale);
//     glm::vec2 rA1 = inv_a * (world[1] * a.scale);
//     glm::vec2 rB0 = inv_b * (world[4] * b.scale);
//     glm::vec2 rB1 = inv_b * (world[5] * b.scale);

//     // If only one contact exists, duplicate
//     if (glm::all(glm::epsilonEqual(rA0, rA1, 1e-6f)))
//         rA1 = rA0;
//     if (glm::all(glm::epsilonEqual(rB0, rB1, 1e-6f)))
//         rB1 = rB0;

//     getManifoldTable()->getRA()[pair.manifoldIndex][0] = rA0;
//     getManifoldTable()->getRA()[pair.manifoldIndex][1] = rA1;
//     getManifoldTable()->getRB()[pair.manifoldIndex][0] = rB0;
//     getManifoldTable()->getRB()[pair.manifoldIndex][1] = rB1;
// }



// }