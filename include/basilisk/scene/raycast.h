#ifndef BSK_RAYCAST_H
#define BSK_RAYCAST_H

#include <basilisk/util/includes.h>

namespace bsk::internal {

class Node;
class Node2D;

// ray cast structs
struct RayCastResult {
    Node* node = nullptr;
    glm::vec3 intersection = glm::vec3(0.0f);
    glm::vec3 normal = glm::vec3(0.0f);
    float distance = std::numeric_limits<float>::infinity();
};

// ray cast structs
struct RayCastResult2D {
    Node2D* node = nullptr;
    glm::vec2 intersection = glm::vec2(0.0f);
    glm::vec2 normal = glm::vec2(0.0f);
    float distance = std::numeric_limits<float>::infinity();
};

}

#endif