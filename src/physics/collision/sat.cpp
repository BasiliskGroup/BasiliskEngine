#include <basilisk/physics/collision/gjk.h>

#include <cmath>

namespace bsk::internal {

void sat(
    const std::vector<glm::vec2>& polyA,
    const std::vector<glm::vec2>& polyB,
    CollisionPair& cp)
{
    float  minOverlap = std::numeric_limits<float>::infinity();
    glm::vec2 mtvAxis = glm::vec2(0.f);

    for (int pass = 0; pass < 2; ++pass)
    {
        const auto& axes = (pass == 0) ? getAxes(polyA) : getAxes(polyB);

        for (const auto& axis : axes)
        {
            auto [minA, maxA] = projectPolygon(polyA, axis);
            auto [minB, maxB] = projectPolygon(polyB, axis);

            const float overlap = getOverlap(minA, maxA, minB, maxB);

            if (overlap < minOverlap)
            {
                minOverlap = overlap;
                mtvAxis    = axis;
            }
        }
    }

    // Orient the normal from B toward A
    glm::vec2 centroidA(0.f), centroidB(0.f);
    for (const auto& v : polyA) centroidA += v;
    for (const auto& v : polyB) centroidB += v;
    centroidA /= static_cast<float>(polyA.size());
    centroidB /= static_cast<float>(polyB.size());

    if (glm::dot(centroidA - centroidB, mtvAxis) < 0.f)
        mtvAxis = -mtvAxis;

    cp.normal = mtvAxis;
}

void findContactPoints(
    const std::vector<glm::vec2>& polyA,
    const std::vector<glm::vec2>& polyB,
    CollisionPair& cp)
{
    auto getBestEdge = [](const std::vector<glm::vec2>& poly,
                          const glm::vec2& dir) -> std::pair<glm::vec2, glm::vec2>
    {
        int   supportIdx = 0;
        float bestDot    = -std::numeric_limits<float>::infinity();
        for (int i = 0; i < (int)poly.size(); ++i)
        {
            float d = glm::dot(poly[i], dir);
            if (d > bestDot) { bestDot = d; supportIdx = i; }
        }

        const int n    = (int)poly.size();
        const int prev = (supportIdx - 1 + n) % n;
        const int next = (supportIdx + 1)     % n;

        glm::vec2 eToPrev = glm::normalize(poly[prev] - poly[supportIdx]);
        glm::vec2 eToNext = glm::normalize(poly[next] - poly[supportIdx]);

        if (std::abs(glm::dot(eToPrev, dir)) <= std::abs(glm::dot(eToNext, dir)))
            return { poly[prev], poly[supportIdx] };
        else
            return { poly[supportIdx], poly[next] };
    };

    // Clips segment [p0, p1] against all half-planes of poly (CCW winding).
    // Returns the number of surviving points (0, 1, or 2).
    auto clipEdgeToPoly = [](
        glm::vec2 p0, glm::vec2 p1,
        const std::vector<glm::vec2>& poly,
        glm::vec2& out0, glm::vec2& out1) -> int
    {
        const int n = (int)poly.size();
        for (int i = 0; i < n; ++i)
        {
            const glm::vec2& v0  = poly[i];
            const glm::vec2& v1  = poly[(i + 1) % n];
            const glm::vec2  edge = v1 - v0;
            // Inward-facing normal for CCW polygon
            const glm::vec2  inward = glm::vec2(-edge.y, edge.x);

            float d0 = glm::dot(p0 - v0, inward);
            float d1 = glm::dot(p1 - v0, inward);

            if (d0 < 0.f && d1 < 0.f) return 0;  // both outside

            if (d0 < 0.f || d1 < 0.f)
            {
                float     t   = d0 / (d0 - d1);
                glm::vec2 mid = p0 + t * (p1 - p0);
                if (d0 < 0.f) p0 = mid;
                else          p1 = mid;
            }
        }

        out0 = p0;
        out1 = p1;

        if (glm::length2(out1 - out0) < EPSILON * EPSILON)
            return 1;

        return 2;
    };

    auto [a0, a1] = getBestEdge(polyA,  cp.normal);
    auto [b0, b1] = getBestEdge(polyB, -cp.normal);

    glm::vec2 clippedA0, clippedA1;
    glm::vec2 clippedB0, clippedB1;

    int numA = clipEdgeToPoly(a0, a1, polyB, clippedA0, clippedA1);
    int numB = clipEdgeToPoly(b0, b1, polyA, clippedB0, clippedB1);

    if (numA == 0)
    {
        // Fall back to deepest point on A along -normal (most into B)
        clippedA0 = glm::dot(a0, -cp.normal) > glm::dot(a1, -cp.normal) ? a0 : a1;
        clippedA1 = clippedA0;
        numA = 1;
    }

    if (numB == 0)
    {
        // Fall back to deepest point on B along normal (most into A)
        clippedB0 = glm::dot(b0, cp.normal) > glm::dot(b1, cp.normal) ? b0 : b1;
        clippedB1 = clippedB0;
        numB = 1;
    }

    if (numA == 1) clippedA1 = clippedA0;
    if (numB == 1) clippedB1 = clippedB0;

    cp.a1 = clippedA0; cp.a2 = clippedA1; cp.numA = numA;
    cp.b1 = clippedB0; cp.b2 = clippedB1; cp.numB = numB;
}

}