#ifndef BSK_EXPAND_TENSOR_H
#define BSK_EXPAND_TENSOR_H

#include <basilisk/util/includes.h>

namespace bsk::internal {

template <typename... T>
void expandTensors(const uint newCapacity, std::vector<T>&... tensors) {
    ( tensors.resize(newCapacity), ... );
}

}

#endif