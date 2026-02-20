#ifndef BSK_MATHS_H
#define BSK_MATHS_H

#include <basilisk/util/includes.h>

namespace bsk::internal {

// Moller-Trumbore ray-triangle intersection.
// Returns true if the ray hits the triangle; t, u, v are set (barycentrics: w = 1 - u - v).
inline bool rayTriangleIntersect(const glm::vec3& origin, const glm::vec3& direction,
                                const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2,
                                float& t, float& u, float& v) {
    const float eps = 1e-7f;
    glm::vec3 edge1 = v1 - v0;
    glm::vec3 edge2 = v2 - v0;
    glm::vec3 h = glm::cross(direction, edge2);
    float a = glm::dot(edge1, h);

    if (std::abs(a) < eps)
        return false;

    float f = 1.0f / a;
    glm::vec3 s = origin - v0;
    u = f * glm::dot(s, h);
    if (u < 0.0f || u > 1.0f)
        return false;

    glm::vec3 q = glm::cross(s, edge1);
    v = f * glm::dot(direction, q);
    if (v < 0.0f || u + v > 1.0f)
        return false;

    t = f * glm::dot(edge2, q);
    return t > eps;
}

// Extract vertex position from flat vertex buffer (position is first 3 floats per vertex).
inline glm::vec3 getMeshVertexPosition(const std::vector<float>& vertices,
                                       const std::vector<unsigned int>& indices,
                                       unsigned int vertexIndex) {
    if (vertices.empty() || indices.empty())
        return glm::vec3(0.0f);

    unsigned int maxIndex = 0;
    for (unsigned int idx : indices)
        if (idx > maxIndex) maxIndex = idx;

    unsigned int numVertices = maxIndex + 1;
    unsigned int stride = static_cast<unsigned int>(vertices.size()) / numVertices;
    if (stride < 3) stride = 3;

    unsigned int offset = vertexIndex * stride;
    return glm::vec3(vertices[offset], vertices[offset + 1], vertices[offset + 2]);
}

void tripleProduct(const glm::vec2& a, const glm::vec2& b, const glm::vec2& c, glm::vec2& o);
void perpTowards(const glm::vec2& v, const glm::vec2& to, glm::vec2& perp);
void transform(const glm::vec2& pos, const glm::mat2x2& mat, glm::vec2& v);
float cross(const glm::vec2& a, const glm::vec2& b);

inline glm::vec2 xy(const glm::vec3& v) noexcept {
    return glm::vec2(v.x, v.y);
}

float triangleArea2(const glm::vec2& a, const glm::vec2& b, const glm::vec2& c);

// True if point p is inside or on the boundary of triangle (a, b, c).
inline bool pointInTriangle2(const glm::vec2& p, const glm::vec2& a, const glm::vec2& b, const glm::vec2& c) {
    float area = triangleArea2(a, b, c);
    if (std::abs(area) < 1e-9f) return false;  // degenerate triangle
    float a1 = triangleArea2(a, b, p);
    float a2 = triangleArea2(b, c, p);
    float a3 = triangleArea2(c, a, p);
    // Point is inside if all sub-areas have the same sign as the triangle area
    bool pos = area > 0;
    return (pos && a1 >= 0 && a2 >= 0 && a3 >= 0) || (!pos && a1 <= 0 && a2 <= 0 && a3 <= 0);
}

std::pair<glm::vec3, glm::vec2> connectSquare(const glm::vec2& a, const glm::vec2& b, float width=0.1f);

}

#endif