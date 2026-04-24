#ifndef BSK_PHYSICS_CELLULAR_COLOR_H
#define BSK_PHYSICS_CELLULAR_COLOR_H

#include <array>

// NOTE these values are copied from intent.wgsl, you must update both at the same time
enum class MaterialIDs {
    EMPTY = 0,
    SOLID = 1,
    WATER = 2,
    GAS = 3,
    WOOD = 4,
    OBSIDIAN = 5,
    FUSE = 6,
    MUD = 7,
    CLAY = 8,
    GLASS = 9,
    METAL = 10,
    PLASTIC = 11,
    SNOW = 12,
    VAPOR = 13,
};

struct Color {
    static constexpr unsigned char STATIC_MAT_BIT = 1u << 4u;
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char mat_id;
    unsigned char on_fire; // 0/1; packed to GPU cell bit 29

    Color() : r(0), g(0), b(0), mat_id(0), on_fire(0) {}
    Color(unsigned char r, unsigned char g, unsigned char b, unsigned char mat_id = 0,
          unsigned char on_fire = 0)
        : r(r), g(g), b(b), mat_id(mat_id), on_fire(on_fire) {}
    Color(unsigned char r, unsigned char g, unsigned char b, unsigned char mat_id,
          unsigned char on_fire, bool is_static)
        : r(r), g(g), b(b), mat_id(static_cast<unsigned char>((mat_id & 0x0Fu) | (is_static ? STATIC_MAT_BIT : 0u))),
          on_fire(on_fire ? 1u : 0u) {}

    static Color White() { return Color(255, 255, 255, 1); }
    static Color Empty() { return Color(0, 0, 0, 0); }
    static Color Sand() { return Color(194, 178, 128, 1); }
    static Color Water() { return Color(155, 234, 255, 1); }

    unsigned char getMatId() const { return static_cast<unsigned char>(mat_id & 0x0Fu); }
    void setMatId(unsigned char value) { mat_id = static_cast<unsigned char>((value & 0x0Fu) | (mat_id & STATIC_MAT_BIT)); }

    unsigned char getOnFire() const { return static_cast<unsigned char>(on_fire ? 1u : 0u); }
    void setOnFire(unsigned char value) { on_fire = static_cast<unsigned char>(value ? 1u : 0u); }

    bool getIsStatic() const { return (mat_id & STATIC_MAT_BIT) != 0u; }
    void setIsStatic(bool value) {
        mat_id = value
            ? static_cast<unsigned char>(mat_id | STATIC_MAT_BIT)
            : static_cast<unsigned char>(mat_id & static_cast<unsigned char>(~STATIC_MAT_BIT));
    }

    std::array<unsigned char, 3> getRgb() const { return {r, g, b}; }
    void setRgb(const std::array<unsigned char, 3>& value) {
        r = value[0];
        g = value[1];
        b = value[2];
    }

    constexpr bool operator==(const Color& other) const noexcept {
        return r == other.r &&
               g == other.g &&
               b == other.b &&
               mat_id == other.mat_id &&
               on_fire == other.on_fire;
    }
};

// NOTE these values are copied from intent.wgsl, you must update both at the same time
inline bool is_fluid(const Color& color) {
    MaterialIDs mat_id = static_cast<MaterialIDs>(color.getMatId());
    switch (mat_id) {
        case MaterialIDs::WATER:
        case MaterialIDs::GAS:
        case MaterialIDs::VAPOR:
            return true;
            
        default:
            return false;
    }
}

#endif