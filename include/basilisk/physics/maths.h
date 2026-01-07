/*
* Copyright (c) 2025 Chris Giles
*
* Permission to use, copy, modify, distribute and sell this software
* and its documentation for any purpose is hereby granted without fee,
* provided that the above copyright notice appear in all copies.
* Chris Giles makes no representations about the suitability
* of this software for any purpose.
* It is provided "as is" without express or implied warranty.
*/

#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

// Math functions using GLM

namespace bsk::internal {

inline float sign(float x)
{
    return x < 0 ? -1.0f : x > 0 ? 1.0f : 0.0f;
}

inline float lengthSq(glm::vec2 v)
{
    return glm::dot(v, v);
}

inline float lengthSq(glm::vec3 v)
{
    return glm::dot(v, v);
}

inline float cross(glm::vec2 a, glm::vec2 b)
{
    return a.x * b.y - a.y * b.x;
}

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

inline glm::vec2 abs(glm::vec2 v)
{
    return glm::abs(v);
}

inline glm::mat2 abs(glm::mat2 a)
{
    return glm::mat2(glm::abs(a[0]), glm::abs(a[1]));
}

// Helper function to get a row from a 2x2 matrix
// GLM matrices are column-major: mat[col][row]
inline glm::vec2 getRow(const glm::mat2& m, int row)
{
    return glm::vec2(m[0][row], m[1][row]);
}

inline glm::mat2 transpose(glm::mat2 a)
{
    return glm::transpose(a);
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

}