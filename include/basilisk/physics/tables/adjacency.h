#ifndef BSK_PHYSICS_GRAPH_ADJACENCY_H
#define BSK_PHYSICS_GRAPH_ADJACENCY_H

#include <basilisk/util/includes.h>

namespace bsk::internal {

class Rigid;

enum class ForceBodyOffset { A, B };

struct ColoredData {
    Rigid* body;
    std::size_t start;
    std::size_t count;
    float mass;
    float moment;
    glm::vec3 inertial;
};

struct ForceEdgeIndices {
    std::size_t force;
    ForceBodyOffset offset;
};

}

#endif