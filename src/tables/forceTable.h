#ifndef FORCE_TABLE_H
#define FORCE_TABLE_H

#include "tables/virtualTable.h"

class ManifoldTable;
class Force;

// NOTE we do not need copy or move constructor as we will only have one of these
class ForceTable : public VirtualTable {
private:
    ManifoldTable* manifoldTable;

    std::vector<Vec3ROWS> J;
    std::vector<Mat3x3ROWS> H;
    std::vector<FloatROWS> C;
    std::vector<FloatROWS> motor;
    std::vector<FloatROWS> stiffness;
    std::vector<FloatROWS> fracture;
    std::vector<FloatROWS> fmax;
    std::vector<FloatROWS> fmin;
    std::vector<FloatROWS> penalty;
    std::vector<FloatROWS> lambda;

    std::vector<Force*> forces;
    std::vector<bool> toDelete;
    std::vector<ForceType> type;

    std::vector<uint> specialIndex;
    std::vector<uint> bodyIndex;
    std::vector<bool> isA;

public:
    ForceTable(uint capacity);
    ~ForceTable();

    void markAsDeleted(uint index);
    void warmstart(float alpha, float gamma);
    
    inline auto& getIsA() { return isA; }
    inline auto& getJ() { return J; }
    inline auto& getH() { return H; }
    inline auto& getC() { return C; }
    inline auto& getFmax() { return fmax; }
    inline auto& getFmin() { return fmin; }
    inline auto& getLambda() { return lambda; }
    inline auto& getStiffness() { return stiffness; }
    inline auto& getFracture() { return fracture; }
    inline auto& getPenalty() { return penalty; }
    inline auto& getMotor() { return motor; }
    inline auto& getToDelete() { return toDelete; }
    inline auto& getBodyIndex() { return bodyIndex; }
    inline auto& getSpecial() { return specialIndex; }
    inline auto& getForces() { return forces; }
    inline auto& getType() { return type; } 
    inline ManifoldTable* getManifoldTable() { return manifoldTable; }

    void reserveManifolds(uint numPairs, uint& forceIndex, uint& manifoldIndex);
    void resize(uint newCapacity) override;
    void compact() override;
    int insert();
};

#endif