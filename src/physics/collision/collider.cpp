#include <basilisk/physics/collision/collider.h>
#include <basilisk/physics/tables/colliderTable.h>
#include <basilisk/physics/solver.h>

namespace bsk::internal {

Collider::Collider(Solver* solver, std::vector<glm::vec2> vertices)
    : table(solver->getColliderTable())
{
    table->insert(this, vertices); // sets index
}

Collider::~Collider() {
    // ColliderTable destructor handles cleanup of this collider
}

std::vector<glm::vec2>& Collider::getVertices() const {
    return this->table->getVertices(this->index);
}

float Collider::getMass(glm::vec2 scale, float density) const {
    return getArea() * scale.x * scale.y * density;
}

float Collider::getMoment(glm::vec2 scale, float density) const {
    return getBaseMoment() * density * scale.x * scale.y * (scale.x * scale.x + scale.y * scale.y) * 0.5f;
}

float Collider::getRadius(glm::vec2 scale) const {
    return glm::length(getHalfDim() * scale);
}

float Collider::getBaseRadius() const {
    return glm::length(getHalfDim());
}

// getters using index automatically
Collider* Collider::getCollider() const {
    return table->getCollider(index);
}

glm::vec2 Collider::getCOM() const {
    return table->getCOM(index);
}

glm::vec2 Collider::getGC() const {
    return table->getGC(index);
}

glm::vec2 Collider::getHalfDim() const {
    return table->getHalfDim(index);
}

float Collider::getArea() const {
    return table->getArea(index);
}

float Collider::getBaseMoment() const {
    return table->getMoment(index);
}

// setters using index automatically
void Collider::setVertices(const std::vector<glm::vec2>& vertices) {
    table->setVerts(index, vertices);
}

void Collider::setCOM(const glm::vec2& com) {
    table->setCOM(index, com);
}

void Collider::setGC(const glm::vec2& gc) {
    table->setGC(index, gc);
}

void Collider::setHalfDim(const glm::vec2& halfDim) {
    table->setHalfDim(index, halfDim);
}

void Collider::setArea(float area) {
    table->setArea(index, area);
}

void Collider::setBaseMoment(float moment) {
    table->setMoment(index, moment);
}

}