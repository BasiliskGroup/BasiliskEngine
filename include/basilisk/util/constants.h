#ifndef CONSTANTS_H
#define CONSTANTS_H

namespace bsk::internal {
    inline constexpr uint ROWS = 4;
    inline constexpr uint MANIFOLD_ROWS = 4;
    inline constexpr uint JOINT_ROWS = 3;
    inline constexpr uint SPRING_ROWS = 1;
    inline constexpr uint NULL_ROWS = 0;
    inline constexpr float PENALTY_MIN = 1000.0f;
    inline constexpr float PENALTY_MAX = 1e9f;
    inline constexpr float STICK_THRESH = 0.02f;

    // collision
    inline constexpr float COLLISION_MARGIN = 1e-6f;
    inline constexpr ushort GJK_ITERATIONS = 15;
    inline constexpr ushort EPA_ITERATIONS = 15;

    inline constexpr bool SHOW_CONTACTS = true;
}

#endif