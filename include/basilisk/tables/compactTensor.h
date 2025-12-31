#ifndef COMPACT_TENSOR_H
#define COMPACT_TENSOR_H

#include <basilisk/util/includes.h>

namespace bsk::internal {

template <typename... T>
void compactTensors(const std::vector<bool>& toDelete, std::size_t size, std::vector<T>&... tensors)
{
    std::size_t dst = 0;

    for (std::size_t src = 0; src < size; ++src) {
        if (!toDelete[src]) {
            if (dst != src) {
                // Use move for efficient transfer
                ((tensors[dst] = std::move(tensors[src])), ...);
            }
            ++dst;
        }
    }
}

std::size_t numValid(const std::vector<bool>& toDelete, const std::size_t size);

}

#endif