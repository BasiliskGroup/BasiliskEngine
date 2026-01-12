#include <basilisk/physics/collision/primative.h>
#include <basilisk/physics/rigid.h>
#include <basilisk/physics/maths.h>
#include <basilisk/util/constants.h>

namespace bsk::internal {

Primative::Primative(glm::vec2 bl, glm::vec2 tr, Rigid* rigid) :
    bl(bl - BVH_MARGIN), 
    tr(tr + BVH_MARGIN),
    parent(nullptr), 
    left(nullptr), 
    right(nullptr), 
    rigid(rigid),
    mass(0.0f),
    radius(0.0f),
    com(0.0f, 0.0f)
{
    updateArea();
}

Primative::Primative(Primative* left, Primative* right) :
    left(left), 
    right(right), 
    parent(nullptr), 
    rigid(nullptr),
    mass(0.0f),
    radius(0.0f),
    com(0.0f, 0.0f)
{
    updateBound();
    updateArea();

    left->setParent(this);
    right->setParent(this);
}

Primative::~Primative() {
    // Only delete children if this node owns them (they're still attached)
    if (left != nullptr) { 
        delete left; 
        left = nullptr; 
    }
    if (right != nullptr) { 
        delete right; 
        right = nullptr; 
    }
    // Don't delete parent or rigid - we don't own them
    parent = nullptr;
    rigid = nullptr;
}

void Primative::updateArea() {
    area = AABBArea(bl, tr);
}

void Primative::updateBound() {
    bl = glm::min(left->bl, right->bl);
    tr = glm::max(left->tr, right->tr);
}

Primative* Primative::getSibling() const {
    if (parent == nullptr) return nullptr;
    return parent->getSibling(this);
}

Primative* Primative::getSibling(const Primative* primative) const {
    if (left == primative) return right;
    if (right == primative) return left;
    return nullptr;  // primative is not a child
}

bool Primative::intersects(const Primative& other) const {
    return AABBIntersect(bl, tr, other.bl, other.tr);
}

bool Primative::contains(const glm::vec2& point) const {
    return AABBContains(bl, tr, point);
}

std::pair<float, Primative*> Primative::findbestSibling(Primative* primative, float inherited) {
    // compute lowest cost and determine if children are a viable option
    float unionArea = AABBArea(bl, tr, primative->bl, primative->tr);
    float cBest = unionArea + inherited;
    float dArea = unionArea - area;
    float cLow = primative->area + dArea + inherited;

    // dense tree, only check one side
    Primative* bestSibling = this;
    if (left == nullptr || cLow > cBest) return { cBest, bestSibling };

    // investigate children
    auto [cLeft, leftSibling] = left->findbestSibling(primative, inherited + dArea);
    if (cLeft < cBest) { cBest = cLeft; bestSibling = leftSibling; }

    auto [cRight, rightSibling] = right->findbestSibling(primative, inherited + dArea);
    if (cRight < cBest) { cBest = cRight; bestSibling = rightSibling; }

    return { cBest, bestSibling };
}

void Primative::query(const glm::vec2& bl, const glm::vec2& tr, std::vector<Rigid*>& results) const {
    if (left == nullptr) {
        results.push_back(rigid);
        return;
    }

    if (AABBIntersect(left->bl, left->tr, bl, tr)) {
        left->query(bl, tr, results);
    }

    if (AABBIntersect(right->bl, right->tr, bl, tr)) {
        right->query(bl, tr, results);
    }
}

void Primative::query(const glm::vec2& point, std::vector<Rigid*>& results) const {
    if (left == nullptr) {
        results.push_back(rigid);
        return;
    }

    if (AABBContains(left->bl, left->tr, point)) {
        left->query(point, results);
    }
    
    if (AABBContains(right->bl, right->tr, point)) {
        right->query(point, results);
    }
}

void Primative::swapChild(Primative* child, Primative* newChild) {
    if (left == child) {
        left = newChild;
    } else {
        right = newChild;
    }
    newChild->parent = this;
    // Note: old child's parent is not nullified - caller should handle if needed
}

void Primative::refitUpward() {
    // For leaf nodes, bounds are already set, just update area
    // For internal nodes, update bounds from children
    if (isLeaf()) {
        updateArea();
    } else {
        updateBound();
        updateArea();
    }
    
    // Continue upward
    if (parent != nullptr) {
        parent->refitUpward();
    }
}

void Primative::getAllPrimatives(std::vector<PrimativeInfo>& results, int level) const {
    // Add this primative
    results.push_back({bl, tr, level});
    
    // Recursively add children
    if (left != nullptr) {
        left->getAllPrimatives(results, level + 1);
    }
    if (right != nullptr) {
        right->getAllPrimatives(results, level + 1);
    }
}

void Primative::computeMassProperties() {
    if (isLeaf()) {
        mass = rigid->getMass();
        com = rigid->getPosition();
    } else {
        left->computeMassProperties();
        right->computeMassProperties();
        mass = left->mass + right->mass;
        com = (left->mass * left->com + right->mass * right->com) / mass;
    }

    radius = 0.5f * glm::length(tr - bl);
}

glm::vec2 Primative::computeGravity(Rigid* rigid) {
    if (isLeaf() && this->rigid == rigid) {
        return glm::vec2(0.0f);
    }

    glm::vec2 d = com - (glm::vec2) rigid->getPosition();
    float len2 = glm::length2(d);

    if (len2 < EPSILON) {
        return glm::vec2(0.0f);
    }

    float len = glm::sqrt(len2);
    if (isLeaf() || radius / len < GRAVITATIONAL_THETA) {
        return GRAVITATIONAL * mass * d / len2;
    }

    return left->computeGravity(rigid) + right->computeGravity(rigid);
}

}

