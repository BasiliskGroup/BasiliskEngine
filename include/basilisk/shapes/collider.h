#ifndef BSK_COLLIDER_H
#define BSK_COLLIDER_H

#include <basilisk/util/includes.h>

namespace bsk::internal {

class Solver;
class ColliderTable;

class Collider {
private:
    Solver* solver;

    // for Table access
    uint index;

public:
    Collider(Solver* solver, std::vector<glm::vec2> verts);
    ~Collider();

    uint getIndex() { return index; }
    void setIndex(uint index) { this->index = index; }

    ColliderTable* getColliderTable();
};

}

#endif