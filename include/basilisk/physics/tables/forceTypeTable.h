#ifndef FORCE_TYPE_TABLE_H
#define FORCE_TYPE_TABLE_H

#include <basilisk/util/includes.h>
#include <basilisk/physics/tables/virtualTable.h>
#include <basilisk/compute/gpuWrapper.hpp>


namespace bsk::internal {

class ForceTable;
class Force;

template<typename T>
class ForceTypeTable : public VirtualTable {
private:
    ForceTable* forceTable;

    std::vector<Force*> forces;
    std::vector<bool> toDelete;
    std::vector<uint32_t> indexMap;
    std::vector<T> data;

public:
    ForceTypeTable(uint32_t capacity, ForceTable* forceTable);
    ~ForceTypeTable() = default;

    void markAsDeleted(uint32_t index);
    void resize(uint32_t newCapacity);
    void compact();
    void remap();
    void insert(Force* force);

    uint32_t getForceIndex(uint32_t typeTableIndex) const { return indexMap[typeTableIndex]; }
    Force* getForce(uint32_t typeTableIndex) const { return forces[typeTableIndex]; }
    T& getData(uint32_t typeTableIndex) { return data[typeTableIndex]; }

    uint32_t getSize() const { return size; }
    uint32_t getCapacity() const { return capacity; }

    // debug
    void printIndices() const;
};

}

#endif