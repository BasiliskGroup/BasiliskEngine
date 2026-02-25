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
    std::vector<std::size_t> indexMap;
    std::vector<T> data;

public:
    ForceTypeTable(std::size_t capacity, ForceTable* forceTable);
    ~ForceTypeTable() = default;

    void markAsDeleted(std::size_t index);
    void resize(std::size_t newCapacity);
    void compact();
    void remap();
    void insert(Force* force);

    std::size_t getForceIndex(std::size_t typeTableIndex) const { return indexMap[typeTableIndex]; }
    Force* getForce(std::size_t typeTableIndex) const { return forces[typeTableIndex]; }
    T& getData(std::size_t typeTableIndex) { return data[typeTableIndex]; }

    std::size_t getSize() const { return size; }
    std::size_t getCapacity() const { return capacity; }

    // debug
    void printIndices() const;
};

}

#endif