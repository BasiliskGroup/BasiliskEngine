#ifndef BSK_LINALG_H
#define BSK_LINALG_H

#include <basilisk/util/includes.h>

namespace bsk::internal {

inline void solve(const glm::mat3x3& a, glm::vec3& x, const glm::vec3& b) {
    // Compute LDL^T decomposition
    float D1 = a[0][0];
    float L21 = a[1][0] / a[0][0];
    float L31 = a[2][0] / a[0][0];
    float D2 = a[1][1] - L21 * L21 * D1;
    float L32 = (a[2][1] - L21 * L31 * D1) / D2;
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
    x[2] = z3;
    x[1] = z2 - L32 * x[2];
    x[0] = z1 - L21 * x[1] - L31 * x[2];
}

}

#endif