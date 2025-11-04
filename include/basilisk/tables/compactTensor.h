#ifndef BSK_COMPACT_TENSOR_H
#define BSK_COMPACT_TENSOR_H

#include <basilisk/util/includes.h>

namespace bsk::internal {

template <typename... T>
void compactTensors(const std::vector<bool>& toDelete, uint size, std::vector<T>&... tensors)
{
    uint dst = 0;

    for (uint src = 0; src < size; ++src) {
        if (!toDelete[src]) {
            if (dst != src) {
                // Assign all tensors at once
                ((tensors[dst] = tensors[src]), ...);
            }
            ++dst;
        }
    }
}

uint numValid(const std::vector<bool>& toDelete, const uint size);

}

#endif