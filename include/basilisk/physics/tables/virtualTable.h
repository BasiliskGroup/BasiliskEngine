#ifndef BSK_VIRTUAL_TABLE_H
#define BSK_VIRTUAL_TABLE_H

#include <basilisk/util/includes.h>
#include <basilisk/compute/gpuWrapper.hpp>

namespace bsk::internal {

/**
 * @brief Base class for structure-of-arrays (SoA) data tables
 * 
 * VirtualTable provides common functionality for managing data stored in a Structure-of-Arrays
 * format, where each property is stored in a separate array. This improves cache locality and
 * enables better vectorization opportunities compared to Array-of-Structures (AoS) layouts.
 * 
 * Provides utilities for resizing, compacting, and managing the lifecycle of table entries.
 */
class VirtualTable {
protected:
    uint32_t size = 0;      ///< Current number of active entries in the table
    uint32_t capacity = 0;  ///< Current capacity of the internal arrays

public:
    /**
     * @brief Resizes the internal arrays to accommodate more entries
     * @param new_capacity New capacity for the internal arrays
     */
    virtual void resize(uint32_t new_capacity) = 0;
    
    /**
     * @brief Compacts the table by removing all marked entries
     * 
     * Removes all entries marked for deletion and updates indices.
     * This is typically an expensive operation and should be called sparingly.
     */
    virtual void compact() = 0;

    /**
     * @brief Gets the current number of active entries in the table
     * @return Current size (number of active entries)
     */
    uint32_t getSize() const { return size; }
    
    /**
     * @brief Gets the current capacity of the internal arrays
     * @return Current capacity
     */
    uint32_t getCapacity() const { return capacity; } 
};

/**
 * @brief Helper function to resize multiple vectors to the same capacity
 * 
 * Uses C++17 fold expressions to efficiently resize all provided vectors.
 * Useful for expanding all arrays in a Structure-of-Arrays table simultaneously.
 * 
 * @tparam T Variadic template parameter for vector element types
 * @param newCapacity New capacity for all vectors
 * @param tensors Variadic list of vectors to resize
 */
template <typename... T>
void expandTensors(const uint32_t newCapacity, std::vector<T>&... tensors) {
    ( tensors.resize(newCapacity), ... );
}

/**
 * @brief Helper function to compact multiple vectors by removing marked entries
 * 
 * Removes entries marked for deletion in toDelete from all provided vectors,
 * using move semantics for efficient transfer. All vectors are compacted
 * to the same active set of indices.
 * 
 * @tparam T Variadic template parameter for vector element types
 * @param toDelete Vector indicating which entries should be removed (true = delete)
 * @param size Current size of the vectors
 * @param tensors Variadic list of vectors to compact
 */
template <typename... T>
void compactTensors(const std::vector<bool>& toDelete, uint32_t size, std::vector<T>&... tensors) {
    uint32_t dst = 0;

    for (uint32_t src = 0; src < size; ++src) {
        if (!toDelete[src]) {
            if (dst != src) {
                // Use move for efficient transfer
                ((tensors[dst] = std::move(tensors[src])), ...);
            }
            ++dst;
        }
    }
}

uint32_t numValid(const std::vector<bool>& toDelete, const uint32_t size);

template <typename... Ts>
void expandGpuBuffers(uint32_t newCapacity, GpuBuffer<Ts>*&... buffers) {
    (
        [&] {
            delete buffers;
            buffers = new GpuBuffer<Ts>(newCapacity);
        }(), ...
    );
}

}

#endif