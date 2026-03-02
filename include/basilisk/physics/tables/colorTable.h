#ifndef BSK_PHYSICS_COLORING_COLOR_TABLE_H
#define BSK_PHYSICS_COLORING_COLOR_TABLE_H

#include <basilisk/util/includes.h>
#include <basilisk/physics/rigid.h>
#include <basilisk/compute/gpuTypes.hpp>

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

class ColorTable {
private: 
    std::vector<ColorBody> colorBodies;
    std::vector<ColorForce> colorForces;

public: 
    ColorTable() = default;
    ~ColorTable() = default;

    void clear();
    void insert(const ColorBody& colorBody);
    void insert(const ColorForce& colorForce);

    void insert(const std::vector<ColorBody>& colorBodies);
    void insert(const std::vector<ColorForce>& colorForces);

    void resizeBodies(uint32_t numBodies) { colorBodies.resize(numBodies); }
    void resizeForces(uint32_t numForces) { colorForces.resize(numForces); }

    ColorBody& getBody(uint32_t index) { return colorBodies[index]; }
    ColorForce& getForce(uint32_t index) { return colorForces[index]; }

    uint32_t getNumBodies() const { return colorBodies.size(); }
    uint32_t getNumForces() const { return colorForces.size(); }

    ColorBody& getBackBody() { return colorBodies.back(); } 
    ColorForce& getBackForce() { return colorForces.back(); }

    std::vector<ColorBody>& getBodies() { return colorBodies; }
    std::vector<ColorForce>& getForces() { return colorForces; }

    bool empty() const { return colorBodies.empty() && colorForces.empty(); }
};

class ColorTableManager {
private:
    std::vector<ColorTable> colorTables;

public: 
    ColorTableManager() = default;
    ~ColorTableManager() = default;

    ColorTable& getColorTable(int index);
    std::vector<ColorBody>& getColorBodies(int index);
    std::vector<ColorForce>& getColorForces(int index);
    
    void clear();

    void insert(std::size_t color, const ColorBody& colorBody);
    void insert(std::size_t color, const ColorForce& colorForce);

    void resize(uint32_t numColors) { colorTables.resize(numColors); }

    uint32_t getNumColors() const { return colorTables.size(); }
};

}

#endif