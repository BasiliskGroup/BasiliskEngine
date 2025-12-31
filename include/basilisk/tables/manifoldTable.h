#ifndef BSK_MANIFOLD_TABLE_H
#define BSK_MANIFOLD_TABLE_H

#include <basilisk/tables/virtualTable.h>

namespace bsk::internal {

class ForceTable;

// NOTE we do not need copy or move constructor as we will only have one of these
class ManifoldTable : public VirtualTable {
private:
    ForceTable* forceTable;

    // xtensor
    std::vector<bool> toDelete;
    std::vector<BskVec2Pair> C0;
    std::vector<BskVec2Pair> rA;
    std::vector<BskVec2Pair> rB;
    std::vector<glm::vec2> normal;
    std::vector<float> friction;
    std::vector<bool> stick;
    std::vector<BskVec2Triplet> simplex;
    std::vector<std::size_t> forceIndex;

    // arrays for holding extra compute space
    std::vector<glm::vec2> tangent;
    std::vector<glm::mat2x2> basis;
    std::vector<BskVec2Pair> rAW;
    std::vector<BskVec2Pair> rBW;
    std::vector<BskFloatROWS> cdA;
    std::vector<BskFloatROWS> cdB;

public:
    ManifoldTable(ForceTable* forceTable, std::size_t capacity);
    ~ManifoldTable() = default;

    void warmstart();

    // getters
    auto& getBasis() { return basis; }
    auto& getTangent() { return tangent; }
    auto& getNormal() { return normal; }
    auto& getRA() { return rA; }
    auto& getRB() { return rB; } 
    auto& getRAW() { return rAW; } 
    auto& getRBW() { return rBW; }
    auto& getC0() { return C0; }
    auto& getStick() { return stick; }
    auto& getForceIndex() { return forceIndex; }
    auto& getSimplex() { return simplex; }
    auto& getFriction() { return friction; }
    auto& getCdA() { return cdA; }
    auto& getCdB() { return cdB; }
    glm::vec2* getSimplexPtr(std::size_t index) { return simplex[index].data(); }

    std::size_t reserve(std::size_t numBodies);
    void resize(std::size_t new_capacity) override;
    void compact() override;
    void remove(std::size_t index);
};

}

#endif