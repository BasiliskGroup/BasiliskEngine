#include <basilisk/physics/collision/collider.h>
#include <basilisk/physics/tables/colliderTable.h>
#include <basilisk/physics/solver.h>

namespace bsk::internal {

Collider::Collider(std::vector<glm::vec2> vertices)
    : table(Solver::getColliderTable())
{
    table->insert(this, vertices); // sets index
}

Collider::~Collider() {
    // ColliderTable destructor handles cleanup of this collider
    markForDeletion();
}

void Collider::markForDeletion() {
    table->markAsDeleted(index);
}

std::vector<glm::vec2>& Collider::getVertices() const {
    return this->table->getVertices(this->index);
}

bool Collider::containsPoint(const glm::vec2& point) const {
    const std::vector<glm::vec2>& verts = getVertices();
    const std::size_t n = verts.size();
    if (n < 3) {
        return false;
    }

    bool inside = false;
    for (std::size_t i = 0, j = n - 1; i < n; j = i++) {
        const glm::vec2& a = verts[j];
        const glm::vec2& b = verts[i];
        const bool straddle = (a.y > point.y) != (b.y > point.y);
        if (straddle) {
            const float t = (point.y - a.y) / (b.y - a.y);
            const float xInt = a.x + t * (b.x - a.x);
            if (point.x < xInt) {
                inside = !inside;
            }
        }
    }
    return inside;
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