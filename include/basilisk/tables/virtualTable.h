#ifndef BSK_VIRTUAL_TABLE_H
#define BSK_VIRTUAL_TABLE_H

#include <basilisk/util/includes.h>
#include <basilisk/tables/compactTensor.h>

namespace bsk::internal {

class VirtualTable {
protected:
    std::size_t size = 0;
    std::size_t capacity = 0;

public:
    virtual void resize(std::size_t new_capacity) = 0;
    virtual void compact() = 0;

    std::size_t getSize() { return size; }
    std::size_t getCapacity() { return capacity; } 
};

template <typename... T>
void expandTensors(const uint newCapacity, std::vector<T>&... tensors) {
    ( tensors.resize(newCapacity), ... );
}

}

#endif