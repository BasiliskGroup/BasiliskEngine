#include <basilisk/physics/cellular/rdp.h>

namespace bsk::internal {

void RDP(const std::vector<glm::vec2>& input, std::vector<glm::vec2>& output, float epsilon) {
    output.clear();

    if (input.size() < 3) {
        output = input;
        return;
    }

    // ---- Step 1: find a stable cut point ----
    // Pick vertex with max distance from centroid (heuristic)
    glm::vec2 centroid(0.0f);
    for (const auto& p : input) centroid += p;
    centroid /= static_cast<float>(input.size());

    int startIdx = 0;
    float maxDist = -1.0f;
    for (int i = 0; i < (int)input.size(); ++i) {
        float d = glm::length2(input[i] - centroid);
        if (d > maxDist) {
            maxDist = d;
            startIdx = i;
        }
    }

    // ---- Step 2: unwrap polygon into polyline ----
    std::vector<glm::vec2> pts;
    pts.reserve(input.size() + 1);

    for (int i = 0; i < (int)input.size(); ++i) {
        pts.push_back(input[(startIdx + i) % input.size()]);
    }
    pts.push_back(pts[0]); // close loop explicitly

    // ---- Step 3: RDP ----
    float epsSq = epsilon * epsilon;
    std::vector<bool> keep(pts.size(), false);

    keep[0] = true;
    keep[pts.size() - 1] = true;

    rdpRecursive(pts, 0, pts.size() - 1, epsSq, keep);

    // ---- Step 4: rebuild output (skip duplicate last point) ----
    for (int i = 0; i < (int)pts.size() - 1; ++i) {
        if (keep[i]) {
            output.push_back(pts[i]);
        }
    }
}

}