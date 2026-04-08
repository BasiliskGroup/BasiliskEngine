#include <basilisk/physics/tables/virtualTable.h>

namespace bsk::internal {

uint32_t numValid(const std::vector<bool>& toDelete, const uint32_t size) {
    uint32_t count = 0;
    for (uint32_t i = 0; i < size; i++) {
        if (toDelete[i] == false) {
            count++;
        }
    }

    return count;
}

}