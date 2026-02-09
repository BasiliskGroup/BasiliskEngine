#ifndef FORCE_TYPE_TABLE_H
#define FORCE_TYPE_TABLE_H

#include <basilisk/util/includes.h>
#include <basilisk/physics/tables/virtualTable.h>

namespace bsk::internal {

class ForceTable;
class Force;

template<typename T>
class ForceTypeTable : public VirtualTable {
private:
    ForceTable* forceTable;

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
};

}

#endif