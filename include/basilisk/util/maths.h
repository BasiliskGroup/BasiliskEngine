#ifndef BSK_MATHS_H
#define BSK_MATHS_H

#include <basilisk/util/includes.h>

namespace bsk::internal {

// ------------------------------------------------------------
// Msc Ops
// ------------------------------------------------------------

inline float sign(float x)
{
    return x < 0 ? -1.0f : x > 0 ? 1.0f : 0.0f;
}

inline float cross(glm::vec2 a, glm::vec2 b)
{
    return a.x * b.y - a.y * b.x;
}

inline void tripleProduct(const glm::vec2& a, const glm::vec2& b, const glm::vec2& c, glm::vec2& o) {
    o = glm::dot(a, c) * b - glm::dot(a, b) * c;
}

inline glm::mat2 abs(glm::mat2 a)
{
    return glm::mat2(glm::abs(a[0]), glm::abs(a[1]));
}

// ------------------------------------------------------------
// Geometric
// ------------------------------------------------------------

inline void perpTowards(const glm::vec2& v, const glm::vec2& to, glm::vec2& perp) {
    // Two possible perpendiculars
    glm::vec2 left  = glm::vec2(-v.y,  v.x);
    glm::vec2 right = glm::vec2( v.y, -v.x);

    // Pick whichever points more toward 'to'
    perp = (glm::dot(left, to) > glm::dot(right, to)) ? left : right;
}

inline bool AABBIntersect(glm::vec2 blA, glm::vec2 trA, glm::vec2 blB, glm::vec2 trB) {
    return (blA.x <= trB.x && trA.x >= blB.x && blA.y <= trB.y && trA.y >= blB.y);
}

inline bool AABBContains(glm::vec2 blA, glm::vec2 trA, glm::vec2 point) {
    return (point.x >= blA.x && point.x <= trA.x && point.y >= blA.y && point.y <= trA.y);
}

inline float AABBArea(glm::vec2 bl, glm::vec2 tr) {
    return (tr.x - bl.x) * (tr.y - bl.y);
}

inline float AABBArea(glm::vec2 blA, glm::vec2 trA, glm::vec2 blB, glm::vec2 trB) {
    glm::vec2 bl = glm::min(blA, blB);
    glm::vec2 tr = glm::max(trA, trB);
    return AABBArea(bl, tr);
}

// Moller-Trumbore ray-triangle intersection.
// Returns true if the ray hits the triangle; t, u, v are set (barycentrics: w = 1 - u - v).
inline bool rayTriangleIntersect(const glm::vec3& origin, const glm::vec3& direction,
    const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2,
    float& t, float& u, float& v) 
{
    const float eps = 1e-7f;
    glm::vec3 edge1 = v1 - v0;
    glm::vec3 edge2 = v2 - v0;
    glm::vec3 h = glm::cross(direction, edge2);
    float a = glm::dot(edge1, h);

    if (std::abs(a) < eps) return false;

    float f = 1.0f / a;
    glm::vec3 s = origin - v0;
    u = f * glm::dot(s, h);
    if (u < 0.0f || u > 1.0f) return false;

    glm::vec3 q = glm::cross(s, edge1);
    v = f * glm::dot(direction, q);
    if (v < 0.0f || u + v > 1.0f) return false;

    t = f * glm::dot(edge2, q);
    return t > eps;
}

// ------------------------------------------------------------
// Linear Algebra
// ------------------------------------------------------------

// Outer product (tensor product): outer(a, b) = a * b^T
// In GLM (column-major), this means column i = b * a[i]
inline glm::mat2 outer(glm::vec2 a, glm::vec2 b)
{
    glm::mat2 result;
    result[0] = b * a.x;  // column 0
    result[1] = b * a.y;  // column 1
    return result;
}

inline glm::mat3 outer(glm::vec3 a, glm::vec3 b)
{
    glm::mat3 result;
    result[0] = b * a.x;  // column 0
    result[1] = b * a.y;  // column 1
    result[2] = b * a.z;  // column 2
    return result;
}

// Helper function to get a row from a 2x2 matrix
// GLM matrices are column-major: mat[col][row]
inline glm::vec2 getRow(const glm::mat2& m, int row)
{
    return glm::vec2(m[0][row], m[1][row]);
}

// Rotation matrix (2D rotation around origin)
// GLM matrices are column-major, so we construct columns
inline glm::mat2 rotation(float angle)
{
    float c = cosf(angle);
    float s = sinf(angle);
    // Rotation matrix [c -s; s c] in row-major becomes
    // [c s; -s c] in column-major (columns first)
    return glm::mat2(
        c, s,    // column 0: (c, s)
        -s, c    // column 1: (-s, c)
    );
}

inline glm::mat3 diagonal(float m00, float m11, float m22)
{
    // GLM matrices are column-major
    return glm::mat3(
        m00, 0.0f, 0.0f,   // column 0
        0.0f, m11, 0.0f,   // column 1
        0.0f, 0.0f, m22    // column 2
    );
}

// Transform 2D point by 3D transform (x, y, angle)
inline glm::vec2 transform(glm::vec3 q, glm::vec2 v)
{
    glm::mat2 R = rotation(q.z);
    return R * v + glm::vec2(q.x, q.y);
}

inline void transform(const glm::vec2& pos, const glm::mat2x2& mat, glm::vec2& v) {
    v = mat * v + pos;
}

inline glm::vec2 rotate(float angle, glm::vec2 v)
{
    return rotation(angle) * v;
}

// Solve linear system Ax = b using LDL^T decomposition
// Note: GLM matrices are column-major, so a[i][j] is column i, row j
inline glm::vec3 solve(glm::mat3 a, glm::vec3 b)
{
    // Compute LDL^T decomposition
    float D1 = a[0][0];  // element at column 0, row 0
    float L21 = a[0][1] / a[0][0];  // element at column 0, row 1
    float L31 = a[0][2] / a[0][0];  // element at column 0, row 2
    float D2 = a[1][1] - L21 * L21 * D1;
    float L32 = (a[1][2] - L21 * L31 * D1) / D2;
    float D3 = a[2][2] - (L31 * L31 * D1 + L32 * L32 * D2);

    // Forward substitution: Solve Ly = b
    float y1 = b.x;
    float y2 = b.y - L21 * y1;
    float y3 = b.z - L31 * y1 - L32 * y2;

    // Diagonal solve: Solve Dz = y
    float z1 = y1 / D1;
    float z2 = y2 / D2;
    float z3 = y3 / D3;

    // Backward substitution: Solve L^T x = z
    glm::vec3 x;
    x.z = z3;
    x.y = z2 - L32 * x.z;
    x.x = z1 - L21 * x.y - L31 * x.z;

    return x;
}

inline float triangleArea2(const glm::vec2& a, const glm::vec2& b, const glm::vec2& c) {
    return (b.x - a.x) * (c.y - a.y) - (c.x - a.x) * (b.y - a.y);
}

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

// ------------------------------------------------------------
// Helper
// ------------------------------------------------------------

// Extract vertex position from flat vertex buffer (position is first 3 floats per vertex).
inline glm::vec3 getMeshVertexPosition(const std::vector<float>& vertices,
    const std::vector<unsigned int>& indices,
    unsigned int vertexIndex) 
{
    if (vertices.empty() || indices.empty()) return glm::vec3(0.0f);

    unsigned int maxIndex = 0;
    for (unsigned int idx : indices)
    if (idx > maxIndex) maxIndex = idx;

    unsigned int numVertices = maxIndex + 1;
    unsigned int stride = static_cast<unsigned int>(vertices.size()) / numVertices;
    if (stride < 3) stride = 3;

    unsigned int offset = vertexIndex * stride;
    return glm::vec3(vertices[offset], vertices[offset + 1], vertices[offset + 2]);
}

inline glm::vec2 xy(const glm::vec3& v) noexcept {
    return glm::vec2(v.x, v.y);
}

inline std::pair<glm::vec3, glm::vec2> connectSquare(const glm::vec2& a, const glm::vec2& b, float width) {
    glm::vec2 delta = b - a;
    float len = glm::length(delta);
    glm::vec2 mid = (a + b) * 0.5f;
    float angle = std::atan2(delta.y, delta.x);
    glm::vec3 pos(mid.x, mid.y, angle);
    glm::vec2 scale(len, width);

    return {pos, scale};
}

}

#endif