#ifndef BSK_PHYSICS_CELLULAR_RDP_H
#define BSK_PHYSICS_CELLULAR_RDP_H

#include <basilisk/util/includes.h>

namespace bsk::internal {

// comfortably between 0.707 and 1.0 to catch jagged unit diagonal slopes
inline constexpr float RDP_EPSILON = 0.85f; 

// Squared distance from point p to segment (a, b)
static float pointSegmentDistSq(const glm::vec2& p,
    const glm::vec2& a,
    const glm::vec2& b)
{
    glm::vec2 ab = b - a;
    float lenSq = glm::dot(ab, ab);

    if (lenSq == 0.0f) {
        return glm::dot(p - a, p - a);
    }

    float t = glm::dot(p - a, ab) / lenSq;
    t = glm::clamp(t, 0.0f, 1.0f);

    glm::vec2 proj = a + t * ab;
    return glm::dot(p - proj, p - proj);
}

// Recursive RDP on index range [start, end]
static void rdpRecursive(const std::vector<glm::vec2>& pts,
    int start, int end,
    float epsSq,
    std::vector<bool>& keep)
{
    float maxDistSq = 0.0f;
    int index = -1;

    for (int i = start + 1; i < end; ++i) {
        float d = pointSegmentDistSq(pts[i], pts[start], pts[end]);
        if (d > maxDistSq) {
            maxDistSq = d;
            index = i;
        }
    }

    if (maxDistSq > epsSq && index != -1) {
        keep[index] = true;
        rdpRecursive(pts, start, index, epsSq, keep);
        rdpRecursive(pts, index, end, epsSq, keep);
    }
}

// Main function
void RDP(const std::vector<glm::vec2>& input, std::vector<glm::vec2>& output, float epsilon=RDP_EPSILON);

}

#endif