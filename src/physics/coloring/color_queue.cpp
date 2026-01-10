#include <basilisk/physics/coloring/color_queue.h>
#include <basilisk/physics/rigid.h>

namespace bsk::internal {

bool RigidComparator::operator()(Rigid* a, Rigid* b) const {
    // Primary comparison: satur (higher is better)
    if (a->getSatur() != b->getSatur()) {
        return a->getSatur() > b->getSatur();  // Reversed for max ordering
    }
    
    // Secondary tiebreaker: degree (higher is better)
    if (a->getDegree() != b->getDegree()) {
        return a->getDegree() > b->getDegree();  // Reversed for max ordering
    }
    
    // Final tiebreaker: pointer address for strict weak ordering
    return a < b;
}

}

