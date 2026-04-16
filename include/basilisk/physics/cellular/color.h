#ifndef BSK_PHYSICS_CELLULAR_COLOR_H
#define BSK_PHYSICS_CELLULAR_COLOR_H

struct Color {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char mat_id;
    unsigned char on_fire; // 0/1; packed to GPU cell bit 29

    Color() : r(0), g(0), b(0), mat_id(0), on_fire(0) {}
    Color(unsigned char r, unsigned char g, unsigned char b, unsigned char mat_id = 0,
          unsigned char on_fire = 0)
        : r(r), g(g), b(b), mat_id(mat_id), on_fire(on_fire) {}

    static Color White() { return Color(255, 255, 255, 1); }
    static Color Empty() { return Color(0, 0, 0, 0); }
    static Color Sand() { return Color(194, 178, 128, 1); }
    static Color Water() { return Color(155, 234, 255, 1); }

    constexpr bool operator==(const Color& other) const noexcept {
        return r == other.r &&
               g == other.g &&
               b == other.b &&
               mat_id == other.mat_id &&
               on_fire == other.on_fire;
    }
};

#endif