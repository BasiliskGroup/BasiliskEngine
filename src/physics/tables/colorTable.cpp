#include <basilisk/physics/tables/colorTable.h>

namespace bsk::internal {

// ------------------------------------------------------------
// ColorTable
// ------------------------------------------------------------
void ColorTable::clear() {
    colorBodies.clear();
    colorForces.clear();
}

void ColorTable::insert(const ColorBody& colorBody) {
    this->colorBodies.push_back(colorBody);
}

void ColorTable::insert(const ColorForce& colorForce) {
    this->colorForces.push_back(colorForce);
}

void ColorTable::insert(const std::vector<ColorBody>& colorBodies) {
    this->colorBodies.insert(this->colorBodies.end(), colorBodies.begin(), colorBodies.end());
}

void ColorTable::insert(const std::vector<ColorForce>& colorForces) {
    this->colorForces.insert(this->colorForces.end(), colorForces.begin(), colorForces.end());
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

void ColorTableManager::insert(std::size_t color, const ColorBody& colorBody) {
    getColorTable(color).insert(colorBody);
}

void ColorTableManager::insert(std::size_t color, const ColorForce& colorForce) {
    getColorTable(color).insert(colorForce);
}

}