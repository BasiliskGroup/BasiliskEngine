#include <basilisk/physics/threading/scratch.h>

namespace bsk::internal {

WorkRange partition(uint32_t totalWork, uint32_t threadID, uint32_t numThreads) {
    uint32_t workPerThread = totalWork / numThreads;
    uint32_t remainingWork = totalWork % numThreads;
    uint32_t start = threadID * workPerThread + (threadID < remainingWork ? threadID : remainingWork);
    uint32_t end = start + workPerThread + (threadID < remainingWork ? 1 : 0);
    return WorkRange{start, end};
}

}