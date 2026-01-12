#ifndef BSK_COLOR_QUEUE_H
#define BSK_COLOR_QUEUE_H

#include <basilisk/util/includes.h>
#include <queue>

namespace bsk::internal {

class Rigid;

/**
 * @brief Comparator for max-heap priority queue based on satur (primary) and degree (secondary tiebreaker)
 * Higher satur and degree values have higher priority
 */
struct RigidComparator {
    bool operator()(Rigid* a, Rigid* b) const;
};

/**
 * @brief Max-heap priority queue for Rigid pointers
 * Top element has highest satur value, with degree as secondary tiebreaker
 */
using ColorQueue = std::priority_queue<Rigid*, std::vector<Rigid*>, RigidComparator>;

}

#endif