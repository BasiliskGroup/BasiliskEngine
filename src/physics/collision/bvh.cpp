#include <basilisk/physics/collision/bvh.h>
#include <basilisk/physics/rigid.h>
#include <basilisk/util/maths.h>

namespace bsk::internal {

BVH::BVH() :
    root(nullptr), size(0), rebuildTimer(0)
{}

BVH::~BVH() {
    if (root != nullptr) { delete root; root = nullptr; }
    size = 0;
}

void BVH::update() {
    rebuildTimer--;
    if (rebuildTimer < 0) {
        rebuild();
        rebuildTimer = 100;
    } else {
        refitAll();
    }
}

void BVH::insert(Rigid* rigid) {
    if (rigid == nullptr) return;

    // create primitive for the rigid
    glm::vec2 bl, tr;
    rigid->getAABB(bl, tr);
    Primitive* primitive = new Primitive(bl, tr, rigid);
    primitives[rigid] = primitive;

    // if the tree is empty, set the root to the primitive
    if (root == nullptr) {
        root = primitive;
        size = 1;
        return;
    }

    // Find the best sibling to attach the new primitive to
    auto [cost, sibling] = root->findbestSibling(primitive, 0.0f);
    Primitive* oldParent = sibling->getParent();
    
    // Create new parent node containing sibling and the new primitive
    // This constructor sets sibling->parent = newParent
    Primitive* newParent = new Primitive(sibling, primitive);
    
    if (sibling == root) {
        // Sibling is root, so newParent becomes the new root
        root = newParent;
        newParent->setParent(nullptr);
    } else {
        // swapChild doesn't check parent pointer, just compares pointers
        oldParent->swapChild(sibling, newParent);
        oldParent->refitUpward();
    }
    
    size++;
}

void BVH::remove(Rigid* rigid) {
    if (rigid == nullptr) return;

    auto it = primitives.find(rigid);
    if (it == primitives.end()) return;
    
    Primitive* primitive = it->second;
    primitives.erase(it);

    // If root, remove the root
    if (root == primitive) {
        root = nullptr;
        size = 0;
        delete primitive;
        return;
    }

    Primitive* parent = primitive->getParent();
    Primitive* sibling = primitive->getSibling();    
    Primitive* grand = parent->getParent();

    // If parent was the root, set the root to the sibling
    if (parent == root) {
        root = sibling;
        sibling->setParent(nullptr);
        parent->setLeft(nullptr);
        parent->setRight(nullptr);
        delete parent;
        delete primitive;
        size--;
        return;
    }
    
    grand->swapChild(parent, sibling);
    grand->refitUpward();
    parent->setLeft(nullptr);
    parent->setRight(nullptr);
    parent->setParent(nullptr);  // Clear parent pointer before deletion
    delete parent;
    delete primitive;
    size--;
}

void BVH::refit(Rigid* rigid) {
    if (rigid == nullptr) return;
    
    auto it = primitives.find(rigid);
    if (it == primitives.end()) return;
    
    Primitive* primitive = it->second;
    glm::vec2 bl, tr;
    rigid->getAABB(bl, tr);
    
    // Check if new AABB (without margin) still fits within old fatted AABB
    if (bl.x >= primitive->getBL().x && bl.y >= primitive->getBL().y &&
        tr.x <= primitive->getTR().x && tr.y <= primitive->getTR().y) {
        // Still fits, just refit the hierarchy upward
        primitive->refitUpward();
        return;
    }
    
    // Doesn't fit, need to remove and reinsert with new margin
    primitive->setBL(bl - BVH_MARGIN);
    primitive->setTR(tr + BVH_MARGIN);
    primitive->refitUpward();
}

void BVH::rebuild() {
    std::vector<Rigid*> allRigids;
    for (auto& [rigid, _] : primitives) {
        allRigids.push_back(rigid);
    }
    
    // Clear tree
    delete root;
    root = nullptr;
    primitives.clear();
    size = 0;
    
    // Reinsert all
    for (Rigid* rigid : allRigids) {
        insert(rigid);
    }
}

void BVH::refitAll() {
    // Collect all rigids first to avoid iterator invalidation
    // (refit() may call remove/insert which modifies the map)
    std::vector<Rigid*> rigidsToRefit;
    rigidsToRefit.reserve(primitives.size());
    for (auto& [rigid, primitive] : primitives) {
        rigidsToRefit.push_back(rigid);
    }
    
    // Now refit each rigid (safe to modify map during this iteration)
    for (Rigid* rigid : rigidsToRefit) {
        // Check if still in map (might have been removed by previous refit)
        if (primitives.find(rigid) != primitives.end()) {
            refit(rigid);
        }
    }
}

std::vector<Rigid*> BVH::query(const glm::vec2& bl, const glm::vec2& tr) const {
    if (root == nullptr) return {};
    std::vector<Rigid*> results;
    root->query(bl, tr, results);
    return results;
}

std::vector<Rigid*> BVH::query(const glm::vec2& point) const {
    if (root == nullptr) return {};
    std::vector<Rigid*> results;
    root->query(point, results);
    return results;
}

std::vector<Rigid*> BVH::query(Rigid* rigid) const {
    if (rigid == nullptr) return {};
    glm::vec2 bl, tr;
    rigid->getAABB(bl, tr);
    return query(bl, tr);
}

std::vector<PrimitiveInfo> BVH::getAllPrimitives() const {
    std::vector<PrimitiveInfo> results;
    if (root != nullptr) {
        root->getAllPrimitives(results, 0);
    }
    return results;
}

void BVH::computeMassProperties() {
    if (root != nullptr) {
        root->computeMassProperties();
    }
}

glm::vec2 BVH::computeGravity(Rigid* rigid) {
    if (root != nullptr) {
        return root->computeGravity(rigid);
    }
    return glm::vec2(0.0f, 0.0f);
}

void BVH::getSandAABB(Rigid* rigid, glm::vec2& bl, glm::vec2& tr) const {
    if (rigid == nullptr) return;
    Primitive* p = primitives.at(rigid);
    bl = p->getBL();
    tr = p->getTR();
}

}