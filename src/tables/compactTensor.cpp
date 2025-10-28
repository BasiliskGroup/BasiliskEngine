#include "tables/compactTensor.h"

/**
 * @brief 
 * 
 * @param toDelete 
 * @param size 
 * @return uint 
 */
uint numValid(const std::vector<bool>& toDelete, const uint size) {
    uint count = 0;
    for (uint i = 0; i < size; i++) {
        if (toDelete[i] == false) {
            count++;
        }
    }

    return count;
}