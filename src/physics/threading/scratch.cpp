#include <basilisk/physics/threading/scratch.h>

namespace bsk::internal {

WorkRange partition(std::size_t totalWork, std::size_t threadID, std::size_t numThreads) {
    std::size_t workPerThread = totalWork / numThreads;
    std::size_t remainingWork = totalWork % numThreads;
    std::size_t start = threadID * workPerThread + (threadID < remainingWork ? threadID : remainingWork);
    std::size_t end = start + workPerThread + (threadID < remainingWork ? 1 : 0);
    return WorkRange{start, end};
}

}