#ifndef BSK_CONSTANTS_H
#define BSK_CONSTANTS_H

#include <thread>

namespace bsk::internal {
    inline constexpr unsigned int ROWS = 4;
    inline constexpr unsigned int MANIFOLD_ROWS = 4;
    inline constexpr unsigned int JOINT_ROWS = 3;
    inline constexpr unsigned int SPRING_ROWS = 1;
    inline constexpr unsigned int NULL_ROWS = 0;
    // inline constexpr float PENALTY_MIN = 1.0f;
    // inline constexpr float PENALTY_MAX = 1e9f;
    // inline constexpr float STICK_THRESH = 0.02f;

    // collision
    // inline constexpr float COLLISION_MARGIN = 1e-6f;
    inline constexpr unsigned short GJK_ITERATIONS = 15;
    inline constexpr unsigned short EPA_ITERATIONS = 15;
    inline constexpr float BVH_MARGIN = 0.1f;

    // inline constexpr bool SHOW_CONTACTS = true;

    inline constexpr float EPSILON = 1e-10f;
    inline constexpr float GRAVITATIONAL = 6.67430e-11f;
    inline constexpr float GRAVITATIONAL_THETA = 0.65f;

    // threading
    inline unsigned int NUM_THREADS = std::max(1u, std::thread::hardware_concurrency());
}

#endif