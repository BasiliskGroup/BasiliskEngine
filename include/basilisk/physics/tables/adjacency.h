#ifndef BSK_PHYSICS_GRAPH_ADJACENCY_H
#define BSK_PHYSICS_GRAPH_ADJACENCY_H

#include <basilisk/util/includes.h>

namespace bsk::internal {

enum class ForceBodyOffset { A, B };

struct ForceRange {
    std::size_t start;
    std::size_t count;
};

struct ForceEdgeIndices {
    std::size_t force;
    ForceBodyOffset offset;
};

}

#endif