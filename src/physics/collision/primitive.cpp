#include <basilisk/physics/collision/primitive.h>
#include <basilisk/physics/rigid.h>
#include <basilisk/util/maths.h>
#include <basilisk/util/constants.h>

namespace bsk::internal {

Primitive::Primitive(glm::vec2 bl, glm::vec2 tr, Rigid* rigid) :
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

Primitive::Primitive(Primitive* left, Primitive* right) :
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

Primitive::~Primitive() {
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

void Primitive::updateArea() {
    area = AABBArea(bl, tr);
}

void Primitive::updateBound() {
    bl = glm::min(left->bl, right->bl);
    tr = glm::max(left->tr, right->tr);
}

Primitive* Primitive::getSibling() const {
    if (parent == nullptr) return nullptr;
    return parent->getSibling(this);
}

Primitive* Primitive::getSibling(const Primitive* primitive) const {
    if (left == primitive) return right;
    if (right == primitive) return left;
    return nullptr;  // primitive is not a child
}

bool Primitive::intersects(const Primitive& other) const {
    return AABBIntersect(bl, tr, other.bl, other.tr);
}

bool Primitive::contains(const glm::vec2& point) const {
    return AABBContains(bl, tr, point);
}

std::pair<float, Primitive*> Primitive::findbestSibling(Primitive* primitive, float inherited) {
    // compute lowest cost and determine if children are a viable option
    float unionArea = AABBArea(bl, tr, primitive->bl, primitive->tr);
    float cBest = unionArea + inherited;
    float dArea = unionArea - area;
    float cLow = primitive->area + dArea + inherited;

    // dense tree, only check one side
    Primitive* bestSibling = this;
    if (left == nullptr || cLow > cBest) return { cBest, bestSibling };

    // investigate children
    auto [cLeft, leftSibling] = left->findbestSibling(primitive, inherited + dArea);
    if (cLeft < cBest) { cBest = cLeft; bestSibling = leftSibling; }

    auto [cRight, rightSibling] = right->findbestSibling(primitive, inherited + dArea);
    if (cRight < cBest) { cBest = cRight; bestSibling = rightSibling; }

    return { cBest, bestSibling };
}

void Primitive::query(const glm::vec2& bl, const glm::vec2& tr, std::vector<Rigid*>& results) const {
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

void Primitive::query(const glm::vec2& point, std::vector<Rigid*>& results) const {
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

void Primitive::swapChild(Primitive* child, Primitive* newChild) {
    if (left == child) {
        left = newChild;
    } else {
        right = newChild;
    }
    newChild->parent = this;
    // Note: old child's parent is not nullified - caller should handle if needed
}

void Primitive::refitUpward() {
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

void Primitive::getAllPrimitives(std::vector<PrimitiveInfo>& results, int level) const {
    // Add this primitive
    results.push_back({bl, tr, level});
    
    // Recursively add children
    if (left != nullptr) {
        left->getAllPrimitives(results, level + 1);
    }
    if (right != nullptr) {
        right->getAllPrimitives(results, level + 1);
    }
}

void Primitive::computeMassProperties() {
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

glm::vec2 Primitive::computeGravity(Rigid* rigid) {
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

