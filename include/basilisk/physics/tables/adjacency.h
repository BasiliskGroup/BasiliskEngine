#ifndef BSK_PHYSICS_GRAPH_ADJACENCY_H
#define BSK_PHYSICS_GRAPH_ADJACENCY_H

#include <basilisk/util/includes.h>

namespace bsk::internal {

class Rigid;

enum class ForceBodyOffset : unsigned long { A, B };

struct ColoredData {
    Rigid* body;
    std::size_t start;
    std::size_t joint, manifold, spring, motor;

    float mass;
    float moment;
    glm::vec3 inertial;

    std::size_t getCount() const { return joint + manifold + spring + motor; }
};

struct ForceEdgeIndices {
    std::size_t force;
    ForceBodyOffset offset;
};

}

#endif