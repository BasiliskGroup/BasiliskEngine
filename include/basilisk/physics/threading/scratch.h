#ifndef BSK_THREADING_SCRATCH_H
#define BSK_THREADING_SCRATCH_H

#include <basilisk/util/includes.h>

namespace bsk::internal {

// stage scratch structs
struct PrimalScratch {
    glm::vec3 rhs;
    glm::mat3x3 lhs;
    glm::mat3x3 GoH;
    glm::vec3 J;
};

struct DualScratch {
    
};

// union
constexpr std::size_t MAX_STAGE_BYTES = std::max({ 
    sizeof(PrimalScratch) 
});
struct alignas(alignof(PrimalScratch)) ThreadScratch { std::byte storage[MAX_STAGE_BYTES]; };

// partitioning
struct WorkRange {
    std::size_t start;
    std::size_t end;
};

// partitioning
WorkRange partition(std::size_t totalWork, std::size_t threadID, std::size_t numThreads);

}

#endif