#ifndef BSK_GJK_H
#define BSK_GJK_H

#include <basilisk/util/includes.h>
#include <basilisk/util/constants.h>

namespace bsk::internal {

// ------------------------------------------------------------
// Structs N Stuff
// ------------------------------------------------------------

// NOTE assumes gc is near pos
struct ConvexShape {
    const std::vector<glm::vec2>& vertices;

    glm::vec2 pos;
    float rot;
    glm::vec2 scale;

    ConvexShape(const std::vector<glm::vec2>& vertices, glm::vec2 pos, float rot, glm::vec2 scale)
        : vertices(vertices), pos(pos), rot(rot), scale(scale) {}
};

using Simplex = std::array<glm::vec2, 3>;

struct PolytopeFace {
    glm::vec2 normal;
    float distance;
    ushort va;
    ushort vb;

    PolytopeFace() = default;
    PolytopeFace(ushort va, ushort vb, glm::vec2 normal, float distance)
        : normal(normal), distance(distance), va(va), vb(vb) {}
};

// add 3 since the simplex starts with 3 vertices
using SupportSet = std::array<ushort, EPA_ITERATIONS + 3>;
using SpArray = std::array<glm::vec2, EPA_ITERATIONS + 3>;
using Polytope = std::array<PolytopeFace, EPA_ITERATIONS + 3>;

// TODO merge simplex and sps into one array
struct CollisionPair {
    // gjk
    Simplex simplex;
    glm::vec2 searchDir;
    uint32_t freeIndex = 0;

    // epa
    SpArray sps;
    SupportSet supportSet;
    Polytope polytope;
    glm::vec2 normal = glm::vec2(0.0f); // world space
    // Sum of all polytope vertices in Minkowski space; centroid = interior / vertexCount
    glm::vec2 interior = glm::vec2(0.0f);

    // sat (world space)
    glm::vec2 a1{}, a2{}, b1{}, b2{};
    int numA = 0;
    int numB = 0;

    CollisionPair() = default;
};

// ------------------------------------------------------------
// Helper Methods
// ------------------------------------------------------------

void addSupport(const ConvexShape& shapeA, const ConvexShape& shapeB, CollisionPair& pair, uint32_t insertIndex);
glm::vec2 getFar(const ConvexShape& shape, const glm::vec2& dir);

// ------------------------------------------------------------
// GJK
// ------------------------------------------------------------

bool gjk(const ConvexShape& shapeA, const ConvexShape& shapeB, CollisionPair& pair);

uint32_t handleSimplex(const ConvexShape& shapeA, const ConvexShape& shapeB, CollisionPair& pair, uint32_t freeIndex);
uint32_t handle0(const ConvexShape& shapeA, const ConvexShape& shapeB, CollisionPair& pair);
uint32_t handle1(const ConvexShape& shapeA, const ConvexShape& shapeB, CollisionPair& pair);
uint32_t handle2(const ConvexShape& shapeA, const ConvexShape& shapeB, CollisionPair& pair);
uint32_t handle3(const ConvexShape& shapeA, const ConvexShape& shapeB, CollisionPair& pair);

// ------------------------------------------------------------
// EPA
// ------------------------------------------------------------

bool epa(const ConvexShape& shapeA, const ConvexShape& shapeB, CollisionPair& pair);
void buildFace(CollisionPair& pair, glm::vec2 interior, ushort indexA, ushort indexB, ushort indexL);
ushort polytopeFront(const Polytope& polytope, ushort numFaces);
ushort insertHorizon(SupportSet& supportSet, ushort spIndex, ushort setSize);
bool discardHorizon(SupportSet& supportSet, ushort spIndex, ushort setSize);
void removeFace(Polytope& polytope, ushort index, ushort numFaces);

// ------------------------------------------------------------
// SAT
// ------------------------------------------------------------

// bool sat(const ConvexShape& shapeA, const ConvexShape& shapeB, CollisionPair& pair);
// glm::vec2 findDeepestPoint(const ConvexShape& shape, const glm::vec2& dir);
// int findDeepestPoints(const ConvexShape& shapeA, const glm::vec2& normal, glm::vec2& first, glm::vec2& second, glm::vec2& last);
// void findExtremes(const ConvexShape& shape, const glm::vec2& dir, glm::vec2& first, glm::vec2& last);
// bool polyLineIntersect(const ConvexShape& shape, const glm::vec2& deep1, const glm::vec2& deep2, glm::vec2& a, glm::vec2& b);

// Projects all vertices of a polygon onto an axis and returns [min, max].
static std::pair<float, float> projectPolygon(
    const std::vector<glm::vec2>& poly,
    const glm::vec2& axis)
{
    float minP =  std::numeric_limits<float>::infinity();
    float maxP = -std::numeric_limits<float>::infinity();
 
    for (const auto& v : poly)
    {
        float p = glm::dot(v, axis);
        if (p < minP) minP = p;
        if (p > maxP) maxP = p;
    }
 
    return { minP, maxP };
}

// Returns the overlap of two 1D intervals [minA,maxA] and [minB,maxB].
// A negative value means no overlap.
static float getOverlap(float minA, float maxA, float minB, float maxB)
{
    return std::min(maxA, maxB) - std::max(minA, minB);
}

// Collects outward-facing edge normals for a CCW-wound polygon.
// For CCW winding, the outward normal of edge (v[i] -> v[i+1]) is the
// right-hand perpendicular: (dy, -dx) normalised.
static std::vector<glm::vec2> getAxes(const std::vector<glm::vec2>& poly)
{
    std::vector<glm::vec2> axes;
    axes.reserve(poly.size());
 
    const std::size_t n = poly.size();
    for (std::size_t i = 0; i < n; ++i)
    {
        const glm::vec2 edge = poly[(i + 1) % n] - poly[i];
        // Outward normal for CCW: rotate edge 90° clockwise
        const glm::vec2 normal = glm::vec2(edge.y, -edge.x);
        const float len = glm::length(normal);
        if (len > 1e-8f)
            axes.push_back(normal / len);
    }
 
    return axes;
}

void sat(
    const std::vector<glm::vec2>& polyA,
    const std::vector<glm::vec2>& polyB,
    CollisionPair& cp);

void findContactPoints(
    const std::vector<glm::vec2>& polyA,
    const std::vector<glm::vec2>& polyB,
    CollisionPair& cp);

}

#endif