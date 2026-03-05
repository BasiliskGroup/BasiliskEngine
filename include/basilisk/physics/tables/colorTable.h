#ifndef BSK_PHYSICS_COLORING_COLOR_TABLE_H
#define BSK_PHYSICS_COLORING_COLOR_TABLE_H

#include <basilisk/util/includes.h>
#include <basilisk/physics/rigid.h>
#include <basilisk/compute/gpuTypes.hpp>
#include <basilisk/compute/gpuWrapper.hpp>

namespace bsk::internal {

struct ColorBounds {
    uint32_t start;
    uint32_t end; // exclusive
};

struct ColorBody {
    Rigid* body;
    uint32_t start;
    uint32_t joint, manifold, spring, motor; // number of each force type
    
    uint32_t getCount() const { return joint + manifold + spring + motor; }
};

struct ColorForce {
    uint32_t special;
    uint32_t bodyIndex; // even though the force table stores the bodies, we need to say which one we are computing for. 
    ForceType type;
    bsk::vec3 jacobianMask; // TODO can be extracted from body table
};

struct ColorTable {
    std::vector<ColorBody> bodies;
    std::vector<ColorForce> forces;
};

struct ColorTableManager {
    std::vector<ColorTable> tables;

    // GpuBuffer<ColorTable> colorBounds;
    // GpuBuffer<ColorBody> colorBodies; // size of body table
    // GpuBuffer<ColorForce> colorForces; // twice the size of force table
};

}

#endif