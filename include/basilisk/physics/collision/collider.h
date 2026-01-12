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
    std::vector<glm::vec2>& getVertices() const;
    float getMass(glm::vec2 scale, float density) const;
    float getMoment(glm::vec2 scale, float density) const;
    float getRadius(glm::vec2 scale) const;
    
    // getters using index automatically
    Collider* getCollider() const;
    glm::vec2 getCOM() const;
    glm::vec2 getGC() const;
    glm::vec2 getHalfDim() const;
    float getArea() const;
    float getBaseMoment() const;
    float getBaseRadius() const;

    // setters
    void setIndex(std::size_t index) { this->index = index; }
    void setVertices(const std::vector<glm::vec2>& vertices);
    void setCOM(const glm::vec2& com);
    void setGC(const glm::vec2& gc);
    void setHalfDim(const glm::vec2& halfDim);
    void setArea(float area);
    void setBaseMoment(float moment);
};

}

#endif