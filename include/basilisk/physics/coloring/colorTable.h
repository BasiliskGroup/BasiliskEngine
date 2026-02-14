#ifndef BSK_PHYSICS_COLORING_COLOR_TABLE_H
#define BSK_PHYSICS_COLORING_COLOR_TABLE_H

#include <basilisk/util/includes.h>
#include <basilisk/physics/tables/adjacency.h>

namespace bsk::internal {

class ColorTable {
private: 
    std::vector<ColoredData> coloredData;
    std::vector<ForceEdgeIndices> forceIndices;

public: 
    ColorTable() = default;
    ~ColorTable() = default;

    void clear();
    void insert(const ColoredData& coloredData);
    void insert(const ForceEdgeIndices& forceIndices);
};

class ColorTableManager {
private:
    std::vector<ColorTable> colorTables;

public: 
    ColorTableManager() = default;
    ~ColorTableManager() = default;

    ColorTable& getColorTable(int index);
    void clear();

    void insert(std::size_t color, const ColoredData& coloredData);
    void insert(std::size_t color, const ForceEdgeIndices& forceIndices);
};

}

#endif