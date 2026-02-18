#include <basilisk/physics/coloring/colorTable.h>

namespace bsk::internal {

// ------------------------------------------------------------
// ColorTable
// ------------------------------------------------------------
void ColorTable::clear() {
    coloredData.clear();
    forceIndices.clear();
}

void ColorTable::insert(const ColoredData& coloredData) {
    this->coloredData.push_back(coloredData);
}

void ColorTable::insert(const ForceEdgeIndices& forceIndices) {
    this->forceIndices.push_back(forceIndices);
}

// ------------------------------------------------------------
// ColorTableManager
// ------------------------------------------------------------
void ColorTableManager::clear() {
    // don't remove tables, only clear them
    for (ColorTable& colorTable : colorTables) {
        colorTable.clear();
    }
}

ColorTable& ColorTableManager::getColorTable(int index) {
    assert(index < colorTables.size());

    // if we don't have a color table for this index, create it
    colorTables.resize(index + 1);

    return colorTables[index];
}

void ColorTableManager::insert(std::size_t color, const ColoredData& coloredData) {
    getColorTable(color).insert(coloredData);
}

void ColorTableManager::insert(std::size_t color, const ForceEdgeIndices& forceIndices) {
    getColorTable(color).insert(forceIndices);
}

}