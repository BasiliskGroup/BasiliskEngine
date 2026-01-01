#ifndef BSK_COLLIDER_H
#define BSK_COLLIDER_H

#include <basilisk/util/includes.h>

namespace bsk::internal {

class Solver;
class ColliderTable;

class Collider {
private:
    ColliderTable* table;
    std::size_t index;

public: 
    Collider(Solver* solver, std::vector<glm::vec2> vertices);
    ~Collider();

    // getters
    std::size_t getIndex() const { return index; }
    ColliderTable* getTable() const { return table; }

    // setters
    void setIndex(std::size_t index) { this->index = index; }
};

}

#endif