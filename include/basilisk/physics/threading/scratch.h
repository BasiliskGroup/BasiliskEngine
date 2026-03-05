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
constexpr uint32_t MAX_STAGE_BYTES = std::max({ 
    sizeof(PrimalScratch) 
});
struct alignas(alignof(PrimalScratch)) ThreadScratch { std::byte storage[MAX_STAGE_BYTES]; };

// partitioning
struct WorkRange {
    uint32_t start;
    uint32_t end;
};

// partitioning
WorkRange partition(uint32_t totalWork, uint32_t threadID, uint32_t numThreads);

}

#endif