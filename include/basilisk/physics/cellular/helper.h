#ifndef BSK_PHYSICS_CELLULAR_HELPER_H
#define BSK_PHYSICS_CELLULAR_HELPER_H

#include <basilisk/util/includes.h>
#include <basilisk/render/material.h>

namespace bsk::internal {

inline constexpr float EDGE_KEY_EPS = 1e-3f;
inline constexpr float BSK_PI = 3.14159265358979323846f;

// TODO, remove this once testing is done
inline Material getCircularColor(int i, int i_max, int j = 1, int j_max = 1) {
    const float pi = BSK_PI;
    const float t = static_cast<float>(i) / static_cast<float>(std::max(i_max, 1));
    const float u = static_cast<float>(j) / static_cast<float>(std::max(j_max, 1));
    return Material(glm::vec3(
        (150.0f + 50.0f * std::sin(2.0f * pi * t + 0.0f * pi / 3.0f) + 50.0f * u) / 255.0f,
        (150.0f + 50.0f * std::sin(2.0f * pi * t + 2.0f * pi / 3.0f) + 50.0f * u) / 255.0f,
        (150.0f + 50.0f * std::sin(2.0f * pi * t + 4.0f * pi / 3.0f) + 50.0f * u) / 255.0f
    ));
}

inline bool vec2Eq(const glm::vec2& a, const glm::vec2& b) {
    return glm::length2(a - b) < 1e-6f;
}

inline std::int64_t quantizeCoord(float v) {
    return static_cast<std::int64_t>(std::llround(static_cast<double>(v) / EDGE_KEY_EPS));
}

inline bool edgeVecEq(const glm::vec2& a, const glm::vec2& b) {
    return quantizeCoord(a.x) == quantizeCoord(b.x) &&
           quantizeCoord(a.y) == quantizeCoord(b.y);
}

// Edge is a line segment
struct Edge {
    glm::vec2 a;
    glm::vec2 b;
};

inline bool operator==(const Edge& x, const Edge& y) {
    return edgeVecEq(x.a, y.a) && edgeVecEq(x.b, y.b);
}

// define structs for identifying edges
struct Vec2Hash {
    std::size_t operator()(const glm::vec2& v) const noexcept {
        const std::int64_t qx = quantizeCoord(v.x);
        const std::int64_t qy = quantizeCoord(v.y);
        return std::hash<std::int64_t>()(qx) ^ (std::hash<std::int64_t>()(qy) << 1);
    }
};

struct Vec2KeyEq {
    bool operator()(const glm::vec2& x, const glm::vec2& y) const noexcept { return edgeVecEq(x, y); }
};

// Custom hash function for Edge
struct EdgeHash {
    std::size_t operator()(const Edge& e) const noexcept {
        std::size_t h1 = Vec2Hash{}(e.a);
        std::size_t h2 = Vec2Hash{}(e.b);
        return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
    }
};

}

#endif