#include <basilisk/physics/tables/virtualTable.h>

namespace bsk::internal {

std::size_t numValid(const std::vector<bool>& toDelete, const std::size_t size) {
    std::size_t count = 0;
    for (std::size_t i = 0; i < size; i++) {
        if (toDelete[i] == false) {
            count++;
        }
    }

    return count;
}

}