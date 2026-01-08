#include <basilisk/physics/coloring/color_queue.h>
#include <basilisk/physics/rigid.h>

namespace bsk::internal {

bool RigidComparator::operator()(Rigid* a, Rigid* b) const {
    // For max-heap: return true if 'a' has lower priority than 'b'
    // Lower priority means it should come after 'b' in the heap
    
    // Primary comparison: satur (higher is better)
    if (a->getSatur() != b->getSatur()) {
        return a->getSatur() < b->getSatur();
    }
    
    // Secondary tiebreaker: degree (higher is better)
    return a->getDegree() < b->getDegree();
}

}

