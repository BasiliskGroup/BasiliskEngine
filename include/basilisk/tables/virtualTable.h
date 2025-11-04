#ifndef BSK_VIRTUAL_TABLE_H
#define BSK_VIRTUAL_TABLE_H

#include <basilisk/util/includes.h>
#include <basilisk/tables/eraseChunks.h>
#include <basilisk/tables/expandTensor.h>
#include <basilisk/tables/compactTensor.h>

namespace bsk::internal {

class VirtualTable {
protected:
    uint size = 0;
    uint capacity = 0;

public:
    virtual void resize(uint new_capacity) = 0;
    virtual void compact() = 0;

    uint getSize() { return size; }
    uint getCapacity() { return capacity; } 
};

}

#endif