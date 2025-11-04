#ifndef EXPAND_TENSOR_H
#define EXPAND_TENSOR_H

#include <basilisk/util/includes.h>

template <typename... T>
void expandTensors(const uint newCapacity, std::vector<T>&... tensors) {
    ( tensors.resize(newCapacity), ... );
}

#endif