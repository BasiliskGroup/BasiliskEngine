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

        pair.manifold->getRA()[0] = ca * a.scale;
        pair.manifold->getRA()[1] = ca * a.scale;
        pair.manifold->getRB()[0] = cb * b.scale;
        pair.manifold->getRB()[1] = cb * b.scale;
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
    pair.manifold->getRA()[0] = a0 * a.scale;
    pair.manifold->getRA()[1] = a1 * a.scale;
    pair.manifold->getRB()[0] = b0 * b.scale;
    pair.manifold->getRB()[1] = b1 * b.scale;
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